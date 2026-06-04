#include "nmea/Nmea0183.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string_view>

using namespace helm::nmea;

int main() {
    ParserState state;
    const auto now = std::chrono::steady_clock::now();

    {
        const auto known = known_sentence_types();
        assert(known.size() == 11);
        assert(find_sentence_type("RMC") != nullptr);
        assert(find_sentence_type("RMC")->type == SentenceType::Rmc);
        assert(find_sentence_type("RMC")->formatter == std::string_view{"RMC"});
        assert(find_sentence_type("RMC")->parse != nullptr);
        assert(find_sentence_type("XXX") == nullptr);
    }

    {
        const auto r = parse_nmea0183_line("$IIMWV,42.0,T,14.8,N,A", state, now);
        assert(r.status == ParseStatus::Ok);
        assert(r.update.has_wind_true);
        assert(std::fabs(r.update.twa_deg - 42.0) < 1e-9);
        assert(std::fabs(r.update.tws_kt - 14.8) < 1e-9);
        assert(!r.update.has_wind_direction);
    }

    {
        const auto r = parse_nmea0183_line("$WIMWD,287.0,T,284.0,M,14.8,N,7.6,M", state, now);
        assert(r.status == ParseStatus::Ok);
        assert(r.update.has_wind_direction);
        assert(!r.update.has_wind_true);
        assert(std::fabs(r.update.twd_deg - 287.0) < 1e-9);
        assert(std::fabs(r.update.tws_kt - 14.8) < 1e-9);
    }

    {
        const auto r = parse_nmea0183_line("$IIDPT,4.20,0.30,50", state, now);
        assert(r.status == ParseStatus::Ok);
        assert(r.update.has_depth);
        assert(std::fabs(r.update.depth_m - 4.50) < 1e-9);
    }

    {
        const auto r = parse_nmea0183_line("$GPHDT,247.5,T", state, now);
        assert(r.status == ParseStatus::Ok);
        assert(r.update.has_heading_true);
        assert(std::fabs(r.update.heading_true_deg - 247.5) < 1e-9);
    }

    {
        const auto r = parse_nmea0183_line("$GPXXX,1,2,3", state, now);
        assert(r.status == ParseStatus::Unsupported);
        assert(r.sentence.talker == "GP");
        assert(r.sentence.formatter == "XXX");
        assert(r.sentence.fields.get(0) == "1");
        assert(r.sentence.fields.get(2) == "3");
    }

    std::cout << "nmea-parser-test passed\n";
    return 0;
}
