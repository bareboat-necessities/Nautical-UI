#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace helm::nmea {

using SteadyTime = std::chrono::steady_clock::time_point;

enum class ParseStatus {
    Ok,
    Empty,
    TooLong,
    BadStart,
    BadChecksum,
    Unsupported,
    Malformed
};

struct FieldList {
    static constexpr std::size_t MaxFields = 64;
    std::array<std::string_view, MaxFields> fields{};
    std::size_t count{0};

    [[nodiscard]] std::string_view get(std::size_t i) const noexcept {
        return i < count ? fields[i] : std::string_view{};
    }
};

struct Sentence {
    char start{'$'};
    std::string_view talker;
    std::string_view formatter;
    FieldList fields;
    std::string_view raw_without_checksum;
    std::string_view raw_line;
};

struct ParserState {
    std::array<std::string, 10> ais_fragments{};
    int ais_expected_fragments{0};
    int ais_received_fragments{0};
    int ais_sequence_id{-1};
    char ais_channel{0};

    void reset_ais();
};

struct TelemetryUpdate {
    SteadyTime timestamp{};

    bool has_position{false};
    double latitude_deg{};
    double longitude_deg{};

    bool has_sog{false};
    double sog_kt{};

    bool has_cog{false};
    double cog_deg{};

    bool has_heading_true{false};
    double heading_true_deg{};

    bool has_heading_mag{false};
    double heading_mag_deg{};

    bool has_depth{false};
    double depth_m{};

    bool has_wind_apparent{false};
    double awa_deg{};
    double aws_kt{};

    bool has_wind_true{false};
    double twa_deg{};
    double tws_kt{};
    double twd_deg{};

    bool has_gps_fix_quality{false};
    int gps_fix_quality{};
    int satellites{};

    bool has_ais_payload{false};
    std::string ais_payload;
    int ais_fill_bits{0};
    char ais_channel{0};
    bool ais_own_vessel{false};
};

struct ParseResult {
    ParseStatus status{ParseStatus::Malformed};
    TelemetryUpdate update{};
    Sentence sentence{};
};

[[nodiscard]] bool tokenize_nmea0183(std::string_view line, Sentence& out);

[[nodiscard]] ParseResult parse_nmea0183_line(
    std::string_view line,
    ParserState& state,
    SteadyTime now);

[[nodiscard]] const char* to_string(ParseStatus status) noexcept;

} // namespace helm::nmea
