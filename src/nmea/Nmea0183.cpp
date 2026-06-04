#include "nmea/Nmea0183.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace helm::nmea {
namespace {

std::string_view trim_crlf(std::string_view s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) {
        s.remove_suffix(1);
    }
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.remove_prefix(1);
    }
    return s;
}

int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool validate_checksum(std::string_view without_start, std::string_view hex) {
    if (hex.size() < 2) return false;
    const int hi = hex_value(hex[0]);
    const int lo = hex_value(hex[1]);
    if (hi < 0 || lo < 0) return false;
    unsigned char sum = 0;
    for (char c : without_start) {
        sum ^= static_cast<unsigned char>(c);
    }
    return sum == static_cast<unsigned char>((hi << 4) | lo);
}

bool parse_double(std::string_view s, double& out) {
    if (s.empty()) return false;
    // from_chars floating support is not complete on all MinGW/libstdc++ combinations, so use strtod safely.
    std::string tmp(s);
    char* end = nullptr;
    const double v = std::strtod(tmp.c_str(), &end);
    if (end == tmp.c_str() || *end != '\0' || !std::isfinite(v)) return false;
    out = v;
    return true;
}

bool parse_int(std::string_view s, int& out) {
    if (s.empty()) return false;
    const char* first = s.data();
    const char* last = s.data() + s.size();
    int value = 0;
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec != std::errc{} || ptr != last) return false;
    out = value;
    return true;
}

double wrap360(double deg) {
    while (deg < 0.0) deg += 360.0;
    while (deg >= 360.0) deg -= 360.0;
    return deg;
}

bool parse_lat_lon(std::string_view value, std::string_view hemi, bool latitude, double& out_deg) {
    double raw = 0.0;
    if (!parse_double(value, raw) || hemi.empty()) return false;

    // NMEA uses ddmm.mmmm for latitude and dddmm.mmmm for longitude.
    // In both cases degrees are floor(raw / 100); the final two integer digits are minutes.
    const int degrees = static_cast<int>(raw / 100.0);
    const double minutes = raw - static_cast<double>(degrees) * 100.0;
    if (minutes < 0.0 || minutes >= 60.0) return false;
    if (latitude && degrees > 90) return false;
    if (!latitude && degrees > 180) return false;

    double signed_deg = static_cast<double>(degrees) + minutes / 60.0;

    const char h = hemi.front();
    if (h == 'S' || h == 'W') signed_deg = -signed_deg;
    else if (h != 'N' && h != 'E') return false;

    out_deg = signed_deg;
    return true;
}

bool is_status_active(std::string_view s) {
    return !s.empty() && (s.front() == 'A' || s.front() == 'D');
}

void parse_rmc(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // RMC: time,status,lat,N/S,lon,E/W,sog,cog,date,...
    if (!is_status_active(s.fields.get(1))) return;

    double lat = 0.0;
    double lon = 0.0;
    if (parse_lat_lon(s.fields.get(2), s.fields.get(3), true, lat) &&
        parse_lat_lon(s.fields.get(4), s.fields.get(5), false, lon)) {
        u.has_position = true;
        u.latitude_deg = lat;
        u.longitude_deg = lon;
    }

    double v = 0.0;
    if (parse_double(s.fields.get(6), v)) {
        u.has_sog = true;
        u.sog_kt = v;
    }
    if (parse_double(s.fields.get(7), v)) {
        u.has_cog = true;
        u.cog_deg = wrap360(v);
    }
}

void parse_gga(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // GGA: time,lat,N/S,lon,E/W,fix_quality,num_sv,hdop,alt,...
    int fix = 0;
    if (parse_int(s.fields.get(5), fix)) {
        u.has_gps_fix_quality = true;
        u.gps_fix_quality = fix;
    }
    parse_int(s.fields.get(6), u.satellites);

    if (fix <= 0) return;

    double lat = 0.0;
    double lon = 0.0;
    if (parse_lat_lon(s.fields.get(1), s.fields.get(2), true, lat) &&
        parse_lat_lon(s.fields.get(3), s.fields.get(4), false, lon)) {
        u.has_position = true;
        u.latitude_deg = lat;
        u.longitude_deg = lon;
    }
}

void parse_vtg(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // VTG: cog_true,T,cog_mag,M,sog_knots,N,sog_kmh,K,mode
    double v = 0.0;
    if (parse_double(s.fields.get(0), v)) {
        u.has_cog = true;
        u.cog_deg = wrap360(v);
    }
    if (parse_double(s.fields.get(4), v)) {
        u.has_sog = true;
        u.sog_kt = v;
    }
}

void parse_hdt(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    double v = 0.0;
    if (parse_double(s.fields.get(0), v)) {
        u.has_heading_true = true;
        u.heading_true_deg = wrap360(v);
    }
}

void parse_hdg(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    double v = 0.0;
    if (parse_double(s.fields.get(0), v)) {
        u.has_heading_mag = true;
        u.heading_mag_deg = wrap360(v);
    }
}

void parse_mwv(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // MWV: angle,reference(R/T),speed,units,status
    if (!is_status_active(s.fields.get(4))) return;

    double angle = 0.0;
    double speed = 0.0;
    if (!parse_double(s.fields.get(0), angle) || !parse_double(s.fields.get(2), speed)) return;

    const auto unit = s.fields.get(3);
    if (!unit.empty()) {
        switch (unit.front()) {
            case 'M': speed *= 1.943844492; break; // m/s to kt
            case 'K': speed *= 0.539956803; break; // km/h to kt
            case 'N': break;
            default: break;
        }
    }

    const auto ref = s.fields.get(1);
    if (!ref.empty() && ref.front() == 'T') {
        u.has_wind_true = true;
        u.twa_deg = angle;
        u.tws_kt = speed;
    } else {
        u.has_wind_apparent = true;
        u.awa_deg = angle;
        u.aws_kt = speed;
    }
}

void parse_mwd(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // MWD: dir_true,T,dir_mag,M,speed_knots,N,speed_mps,M
    double dir = 0.0;
    double speed = 0.0;
    if (parse_double(s.fields.get(0), dir) && parse_double(s.fields.get(4), speed)) {
        u.has_wind_direction = true;
        u.twd_deg = wrap360(dir);
        u.tws_kt = speed;
    }
}

void parse_dpt(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // DPT: depth,offset,range
    double depth = 0.0;
    double offset = 0.0;
    if (parse_double(s.fields.get(0), depth)) {
        parse_double(s.fields.get(1), offset);
        u.has_depth = true;
        u.depth_m = depth + offset;
    }
}

void parse_dbt(const Sentence& s, ParserState&, TelemetryUpdate& u) {
    // DBT: feet,f,meters,M,fathoms,F
    double depth_m = 0.0;
    if (parse_double(s.fields.get(2), depth_m)) {
        u.has_depth = true;
        u.depth_m = depth_m;
    }
}

void parse_vdm_vdo(const Sentence& s, ParserState& state, TelemetryUpdate& u, bool own_vessel) {
    // VDM/VDO: total,number,seq,channel,payload,fill
    int total = 0;
    int number = 0;
    int fill = 0;
    if (!parse_int(s.fields.get(0), total) || !parse_int(s.fields.get(1), number)) return;
    if (total <= 0 || total > static_cast<int>(state.ais_fragments.size()) || number <= 0 || number > total) return;

    const auto seq = s.fields.get(2);
    const auto channel = s.fields.get(3);
    const auto payload = s.fields.get(4);
    parse_int(s.fields.get(5), fill);

    if (total == 1) {
        u.has_ais_payload = true;
        u.ais_payload = std::string(payload);
        u.ais_fill_bits = fill;
        u.ais_channel = channel.empty() ? '\0' : channel.front();
        u.ais_own_vessel = own_vessel;
        return;
    }

    int seq_id = -1;
    parse_int(seq, seq_id);
    if (number == 1) {
        state.reset_ais();
        state.ais_expected_fragments = total;
        state.ais_sequence_id = seq_id;
        state.ais_channel = channel.empty() ? '\0' : channel.front();
    }

    if (state.ais_expected_fragments != total || state.ais_sequence_id != seq_id) {
        state.reset_ais();
        return;
    }

    state.ais_fragments[static_cast<std::size_t>(number - 1)] = std::string(payload);
    state.ais_received_fragments++;

    if (state.ais_received_fragments == state.ais_expected_fragments) {
        std::string assembled;
        for (int i = 0; i < state.ais_expected_fragments; ++i) {
            assembled += state.ais_fragments[static_cast<std::size_t>(i)];
        }
        u.has_ais_payload = true;
        u.ais_payload = std::move(assembled);
        u.ais_fill_bits = fill;
        u.ais_channel = state.ais_channel;
        u.ais_own_vessel = own_vessel;
        state.reset_ais();
    }
}

void parse_vdm(const Sentence& s, ParserState& state, TelemetryUpdate& u) {
    parse_vdm_vdo(s, state, u, false);
}

void parse_vdo(const Sentence& s, ParserState& state, TelemetryUpdate& u) {
    parse_vdm_vdo(s, state, u, true);
}

constexpr std::array<SentenceTypeDefinition, 11> KnownSentenceTypes{{
    {SentenceType::Dbt, "DBT", parse_dbt},
    {SentenceType::Dpt, "DPT", parse_dpt},
    {SentenceType::Gga, "GGA", parse_gga},
    {SentenceType::Hdg, "HDG", parse_hdg},
    {SentenceType::Hdt, "HDT", parse_hdt},
    {SentenceType::Mwd, "MWD", parse_mwd},
    {SentenceType::Mwv, "MWV", parse_mwv},
    {SentenceType::Rmc, "RMC", parse_rmc},
    {SentenceType::Vdm, "VDM", parse_vdm},
    {SentenceType::Vdo, "VDO", parse_vdo},
    {SentenceType::Vtg, "VTG", parse_vtg},
}};


} // namespace

void ParserState::reset_ais() {
    for (auto& s : ais_fragments) {
        s.clear();
    }
    ais_expected_fragments = 0;
    ais_received_fragments = 0;
    ais_sequence_id = -1;
    ais_channel = 0;
}

bool tokenize_nmea0183(std::string_view line, Sentence& out) {
    line = trim_crlf(line);
    out = Sentence{};
    out.raw_line = line;

    if (line.empty() || line.size() > 120) return false;
    if (line.front() != '$' && line.front() != '!') return false;

    out.start = line.front();
    line.remove_prefix(1);

    const auto star = line.find('*');
    std::string_view body = line;
    if (star != std::string_view::npos) {
        body = line.substr(0, star);
        const auto checksum = line.substr(star + 1);
        if (!validate_checksum(body, checksum)) return false;
    }

    out.raw_without_checksum = body;

    const auto comma = body.find(',');
    const auto head = comma == std::string_view::npos ? body : body.substr(0, comma);
    if (head.size() < 3) return false;

    out.talker = head.substr(0, 2);
    out.formatter = head.substr(2);

    std::string_view rest = comma == std::string_view::npos ? std::string_view{} : body.substr(comma + 1);
    while (!rest.empty() && out.fields.count < FieldList::MaxFields) {
        const auto pos = rest.find(',');
        if (pos == std::string_view::npos) {
            out.fields.fields[out.fields.count++] = rest;
            rest = {};
        } else {
            out.fields.fields[out.fields.count++] = rest.substr(0, pos);
            rest.remove_prefix(pos + 1);
            if (rest.empty() && out.fields.count < FieldList::MaxFields) {
                out.fields.fields[out.fields.count++] = {};
            }
        }
    }

    return true;
}

std::span<const SentenceTypeDefinition> known_sentence_types() noexcept {
    return KnownSentenceTypes;
}

const SentenceTypeDefinition* find_sentence_type(std::string_view formatter) noexcept {
    const auto known = known_sentence_types();
    const auto it = std::find_if(known.begin(), known.end(), [formatter](const SentenceTypeDefinition& candidate) {
        return candidate.formatter == formatter;
    });
    return it == known.end() ? nullptr : &*it;
}

ParseResult parse_nmea0183_line(std::string_view line, ParserState& state, SteadyTime now) {
    ParseResult result;
    result.update.timestamp = now;

    if (trim_crlf(line).empty()) {
        result.status = ParseStatus::Empty;
        return result;
    }
    if (trim_crlf(line).size() > 120) {
        result.status = ParseStatus::TooLong;
        return result;
    }
    const auto t = trim_crlf(line);
    if (t.front() != '$' && t.front() != '!') {
        result.status = ParseStatus::BadStart;
        return result;
    }

    Sentence s;
    if (!tokenize_nmea0183(line, s)) {
        result.status = ParseStatus::BadChecksum;
        return result;
    }

    result.sentence = s;

    if (const auto* sentence_type = find_sentence_type(s.formatter)) {
        sentence_type->parse(s, state, result.update);
        result.status = ParseStatus::Ok;
    } else {
        // Unknown sentences are still tokenized into result.sentence for callers that
        // want generic field access, but they do not produce telemetry updates.
        result.status = ParseStatus::Unsupported;
    }
    return result;
}

const char* to_string(ParseStatus status) noexcept {
    switch (status) {
        case ParseStatus::Ok: return "Ok";
        case ParseStatus::Empty: return "Empty";
        case ParseStatus::TooLong: return "TooLong";
        case ParseStatus::BadStart: return "BadStart";
        case ParseStatus::BadChecksum: return "BadChecksum";
        case ParseStatus::Unsupported: return "Unsupported";
        case ParseStatus::Malformed: return "Malformed";
    }
    return "Unknown";
}

} // namespace helm::nmea
