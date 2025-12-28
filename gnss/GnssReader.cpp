#include "GnssReader.h"
#include "../utils/StringUtils.h"

#include <boost/asio/read_until.hpp>

#include "../event/EventDispatcher.h"
#include "../logging/Logger.h"
#include "../utils/NMEAUtils.h"

GnssReader::GnssReader(boost::asio::io_context& ioCtx, const std::string& port): serialPort_(ioCtx) {
    Logger::instance().info("GnssReader", "Initializing GnssReader on port " + port);
    boost::system::error_code ec;
    serialPort_.open(port, ec);
    if(ec) {
        Logger::instance().error("GnssReader", "Error opening GNSS serial port: " + ec.message());
        return;
    }
    readOperation();
}

void GnssReader::readOperation() {
    boost::asio::async_read_until(serialPort_, buffer_, "\r\n", [this](const boost::system::error_code& ec,
        const std::size_t length) {
        readHandler(ec, length);
    });
}

void GnssReader::readHandler(const boost::system::error_code &ec, const std::size_t length) {
    Logger::instance().trace("GnssReader", "Read " + std::to_string(length) + " bytes");
    if(!ec) {
        std::string line;
        std::istream is(&buffer_);
        std::getline(is, line);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        handlePacket(line);
    } else {
        Logger::instance().error("GnssReader", "Error receiving data from serial port: " + ec.message());
    }
    readOperation();
}

void GnssReader::handlePacket(const std::string& line) {
    const size_t splitPos = line.find_first_of(',');
    const size_t starPos = line.find_last_of('*');
    if(splitPos == std::string::npos || splitPos == 0) {
        Logger::instance().warn("GnssReader", "Invalid line format, cannot parse token");
        Logger::instance().debug("GnssReader", "LINE: " + line);
        return;
    }
    const std::string token = line.substr(0, splitPos);
    if(token.at(0) != '$'){
        Logger::instance().warn("GnssReader", "Invalid token start char: " + token );
        Logger::instance().debug("GnssReader", "LINE: " + line);
        return;
    }
    if(line.at(line.size()-3) != '*') {
        Logger::instance().warn("GnssReader", "No checksum in " + token + " message");
        Logger::instance().debug("GnssReader", "LINE: " + line);
        return;
    }
    if (!validateChecksum(line)) {
        Logger::instance().warn("GnssReader", "Invalid checksum in " + token + " message");
        Logger::instance().debug("GnssReader", "LINE: " + line);
        return;
    }
    //TODO: add missing handlers
    std::string sentenceId;
    const std::string sentence = line.substr(splitPos+1, starPos-7);
    if (token == "$PUBX") {
        handleUbx(sentence);
        return;
    }
    const std::string talker = token.substr(1, 2);
    sentenceId = token.substr(3, 3);
    Logger::instance().trace("GnssReader", line);
    switch(stringHash(sentenceId.c_str())) {
        case stringHash("RMC"): {
            //Probably noop this
            handleRmc(talker, sentence);
            break;
        }
        case stringHash("GGA"): {
            handleGga(talker, sentence);
            break;
        }
        case stringHash("GSA"): {
            handleGsa(talker, sentence);
            break;
        }
        case stringHash("GSV"): {
            //Probably noop this
            handleGsv(talker, sentence);
            break;
        }
        case stringHash("GLL"): {
            //Probably noop this
            handleGll(talker, sentence);
            break;
        }
        case stringHash("VTG"): {
            handleVtg(talker, sentence);
            break;
        }
        case stringHash("TXT"): {
            handleTxt(sentence);
            break;
        }
        default:
        Logger::instance().debug("GnssReader", "Unknown token: " + token);
    }
}

bool GnssReader::validateChecksum(const std::string &line) {
    if (line.empty() || line[0] != '$')
        return false;

    const auto star = line.find_last_of('*');
    if (star == std::string::npos || star + 2 >= line.size())
        return false;

    unsigned char checksum = 0;
    for (size_t i = 1; i < star; ++i)
        checksum ^= static_cast<unsigned char>(line[i]);

    const auto hex = line.substr(star + 1, 2);
    if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1]))
        return false;

    const unsigned int expected = std::stoul(hex, nullptr, 16);

    return checksum == expected;
}


void GnssReader::handleUbx(const std::string& sentence) {
    switch(const auto split = splitString(sentence, ','); stringHash(split[0].c_str())){
        case stringHash("00"): {
            //Position
            const double lat = nmeaPositionToDecimal(split[2], split[3]);
            const double lon = nmeaPositionToDecimal(split[4], split[5]);
            const double alt = strtod(split[6].c_str(), nullptr);
            const std::string navStat = split[7];
            const double hAcc = strtod(split[8].c_str(), nullptr);
            const double vAcc = strtod(split[9].c_str(), nullptr);
            const double spd = strtod(split[10].c_str(), nullptr);
            const double hdg = strtod(split[11].c_str(), nullptr);
            const double vVel = -strtod(split[12].c_str(), nullptr);
            const double ageC = strtod(split[13].c_str(), nullptr);
            const double hdop = strtod(split[14].c_str(), nullptr);

            const auto ev = std::make_shared<GNSSPositionEvent>();
            ev->latitude = lat;
            ev->longitude = lon;
            ev->altitude = alt;
            ev->hdop = hdop;
            ev->hAccuracy = hAcc;
            ev->vAccuracy = vAcc;
            ev->speed = spd;
            ev->heading = hdg;
            ev->vVelocity = vVel;
            ev->correctionAge = ageC;
            ev->constellation = COMBINED;
            EventDispatcher::instance().dispatchAsync(ev);
            break;
        }
        case stringHash("03"): {
            //TODO: Sat Status
            break;
        }
        case stringHash("04"): {
            //TODO: Time of day
            break;
        }
    default:
        Logger::instance().debug("GnssReader", "Unknown UBX token: " + split[0]);
    }
}

void GnssReader::handleRmc(const std::string& talker, const std::string& sentence) {
    const auto split = splitString(sentence, ',');

    const double lat = nmeaPositionToDecimal(split[2], split[3]);
    const double lon = nmeaPositionToDecimal(split[4], split[5]);
    const double spd = strtod(split[6].c_str(), nullptr);
    const double hdg = strtod(split[7].c_str(), nullptr);

    const auto ev = std::make_shared<GNSSPositionEvent>();
    ev->constellation = constellationFromTalker(talker);
    ev->latitude = lat;
    ev->longitude = lon;
    ev->speed = spd;
    ev->heading = hdg;
    EventDispatcher::instance().dispatchAsync(ev);
}

void GnssReader::handleGga(const std::string& talker, const std::string& sentence) {
    const auto split = splitString(sentence, ',');
    //TOD - split[0]
    const double lat = nmeaPositionToDecimal(split[1], split[2]);
    const double lon = nmeaPositionToDecimal(split[3], split[4]);
    const auto quality = static_cast<N183GNSSQualityIndicator>(strtoul(split[5].c_str(), nullptr, 10));
    const bool valid = quality != INVALID && quality != NA;
    const unsigned int svs = strtoul(split[6].c_str(), nullptr, 10);
    const double hdop = strtod(split[7].c_str(), nullptr);
    const double height = strtod(split[8].c_str(), nullptr);
    const double geoidSeparation = strtod(split[10].c_str(), nullptr);

    if (valid){
        const auto ev = std::make_shared<GNSSPositionEvent>();
        ev->constellation = constellationFromTalker(talker);
        ev->latitude = lat;
        ev->longitude = lon;
        ev->altitude = height;
        ev->hdop = hdop;
        //TODO: Expand event to include sat count etc
        EventDispatcher::instance().dispatchAsync(ev);
    } else {
        Logger::instance().warn("GnssReader", "Position not valid: " + talker + "Gga");
    }
}

void GnssReader::handleGsa(const std::string& talker, const std::string& sentence) {
    //TODO: Check against real data and build from there
}

void GnssReader::handleGsv(const std::string& talker, const std::string& sentence) {
    //TODO: Implement GSV
    //TODO: Wait until all messages have been received then parse and send the response
    const auto ev = std::make_shared<GNSSSatellitesEvent>();
    ev->constellation = constellationFromTalker(talker);

    EventDispatcher::instance().dispatchAsync(ev);
}

void GnssReader::handleGll(const std::string& talker, const std::string& sentence) {
    const auto split = splitString(sentence, ',');
    const double lat = nmeaPositionToDecimal(split[0], split[1]);
    const double lon = nmeaPositionToDecimal(split[2], split[3]);
    //TOD - split[4]

    if (split[5] == "A"){
        const auto ev = std::make_shared<GNSSPositionEvent>();
        ev->constellation = constellationFromTalker(talker);
        ev->latitude = lat;
        ev->longitude = lon;
        EventDispatcher::instance().dispatchAsync(ev);
    } else {
        Logger::instance().warn("GnssReader", "Position not valid: " + talker + "GLL");
    }
}

void GnssReader::handleVtg(const std::string& talker, const std::string& sentence) {
    const auto split = splitString(sentence, ',');
    const double trackDegTrue = strtod(split[0].c_str(), nullptr);
    double trackDegMag = strtod(split[2].c_str(), nullptr);
    double spdKts = strtod(split[4].c_str(), nullptr);
    const double speedKph = strtod(split[6].c_str(), nullptr);
    if (split[8] != "N") {
        const auto ev = std::make_shared<GNSSPositionEvent>();
        ev->constellation = constellationFromTalker(talker);
        ev->heading = trackDegTrue;
        ev->speed = speedKph;
        EventDispatcher::instance().dispatchAsync(ev);
    } else {
        Logger::instance().warn("GnssReader", "Course not valid: " + talker + "VTG");
    }
}

void GnssReader::handleTxt(const std::string& line) {
    switch (const auto split = splitString(line, ','); stringHash(split[2].c_str())) {
        case stringHash("00"): {
            Logger::instance().error("GnssReader::handleTxt", split[3]);
            break;
        }
        case stringHash("01"): {
            Logger::instance().warn("GnssReader::handleTxt", split[3]);
            break;
        }
        case stringHash("02"):
        case stringHash("03"): {
            Logger::instance().info("GnssReader::handleTxt", split[3]);
            break;
        }
        default:
            Logger::instance().info("GnssReader::handleTxt", "Unknown Text message level " + split[3]);
    }
}

GNSSSatelliteConstellation GnssReader::constellationFromTalker(const std::string& talker) {
    switch (stringHash(talker.c_str())){
        case stringHash("GN"):{
            return COMBINED;
        }
        case stringHash("GP"):{
            return GPS;
        }
        case stringHash("GL"):{
            return GLONASS;
        }
        case stringHash("GA"):{
            return GALILEO;
        }
        case stringHash("GB"):{
            return BEIDOU;
        }
        case stringHash("GI"):{
            return NAVIC;
        }
        case stringHash("IN"):{
            return INS;
        }
        default:
        return UNKNOWN;
    }
}


