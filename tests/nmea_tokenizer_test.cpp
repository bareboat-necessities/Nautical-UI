#include "nmea/Nmea0183.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

using namespace helm::nmea;

int main() {
    {
        Sentence s;
        const std::string no_checksum = "$GPRMC,143212.00,A,4043.1234,N,07400.5678,W,6.4,048.2,020626,,,A";
        assert(tokenize_nmea0183(no_checksum, s));
        assert(s.start == '$');
        assert(s.talker == "GP");
        assert(s.formatter == "RMC");
        assert(s.fields.get(1) == "A");
        assert(s.fields.get(6) == "6.4");
    }

    {
        ParserState state;
        const auto now = std::chrono::steady_clock::now();
        const std::string line = "$GPRMC,143212.00,A,4043.1234,N,07400.5678,W,6.4,048.2,020626,,,A";
        const auto r = parse_nmea0183_line(line, state, now);
        assert(r.status == ParseStatus::Ok);
        assert(r.update.has_position);
        assert(r.update.has_sog);
        assert(r.update.has_cog);
        assert(std::fabs(r.update.latitude_deg - 40.7187233333) < 1e-9);
        assert(std::fabs(r.update.longitude_deg - (-74.0094633333)) < 1e-9);
        assert(r.update.sog_kt > 6.3 && r.update.sog_kt < 6.5);
    }

    {
        ParserState state;
        const auto now = std::chrono::steady_clock::now();
        const std::string line = "!AIVDM,1,1,,A,15Muq?002>G?svP00<:O?vN60<0,0";
        const auto r = parse_nmea0183_line(line, state, now);
        assert(r.status == ParseStatus::Ok);
        assert(r.update.has_ais_payload);
        assert(!r.update.ais_own_vessel);
        assert(r.update.ais_channel == 'A');
    }

    std::cout << "nmea-tokenizer-test passed\n";
    return 0;
}
