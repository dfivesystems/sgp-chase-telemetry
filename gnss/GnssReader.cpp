#include "GnssReader.h"
#include "../utils/StringUtils.h"

#include <boost/asio/read_until.hpp>

#include "../event/Event.h"
#include "../event/EventDispatcher.h"
#include "../utils/NMEAUtils.h"
#include "../logging/Logger.h"

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
    boost::asio::async_read_until(serialPort_, buffer_, "\r\n", [this](const boost::system::error_code ec, std::size_t length) { readHandler(ec, length); });
}

void GnssReader::readHandler(const boost::system::error_code &ec, std::size_t length) {
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
    size_t splitPos = line.find_first_of(',');
    size_t starPos = line.find_last_of('*');
    if(splitPos == std::string::npos || splitPos == 0) {
        Logger::instance().warn("GnssReader", "Invalid line format, cannot parse token");
        Logger::instance().debug("GnssReader", "LINE: " + line);
        return;
    }
    std::string token = line.substr(0, splitPos);
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
    std::string talker;
    std::string sentenceId;
    std::string sentence = line.substr(splitPos+1, starPos-7);
    if (token == "$PUBX") {
        handleUbx(sentence);
    }
    talker = token.substr(1, 2);
    sentenceId = token.substr(3, 3);
    switch(stringHash(sentenceId.c_str())) {
        case stringHash("RMC"): {
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
            handleGsv(talker, sentence);
            break;
        }
        case stringHash("GLL"): {
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

    auto star = line.find_last_of('*');
    if (star == std::string::npos || star + 2 >= line.size())
        return false;

    unsigned char checksum = 0;
    for (size_t i = 1; i < star; ++i)
        checksum ^= static_cast<unsigned char>(line[i]);

    auto hex = line.substr(star + 1, 2);
    if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1]))
        return false;

    unsigned int expected = std::stoul(hex, nullptr, 16);

    return checksum == expected;
}


void GnssReader::handleUbx(const std::string& line) {
    auto split = splitString(line, ',');
    switch(stringHash(split[0].c_str())){
        case stringHash("00"): {
            //Position
            double lat = nmeaPositionToDecimal(split[2], split[3]);
            double lon = nmeaPositionToDecimal(split[4], split[5]);
            double alt = strtod(split[6].c_str(), nullptr);
            std::string navStat = split[7];
            double hAcc = strtod(split[8].c_str(), nullptr);
            double vAcc = strtod(split[9].c_str(), nullptr);
            double spd = strtod(split[10].c_str(), nullptr);
            double hdg = strtod(split[11].c_str(), nullptr);
            double vVel = -strtod(split[12].c_str(), nullptr);
            double ageC = strtod(split[13].c_str(), nullptr);
            double hdop = strtod(split[14].c_str(), nullptr);

            auto ev = std::make_shared<GNSSPositionEvent>();
            ev->setLatitude(lat);
            ev->setLongitude(lon);
            ev->setAltitude(alt);
            ev->setHdop(hdop);
            ev->setHAccuracy(hAcc);
            ev->setVAccuracy(vAcc);
            ev->setSpeed(spd);
            ev->setHeading(hdg);
            ev->setVVelocity(vVel);
            ev->setCorrectionAge(ageC);
            EventDispatcher::instance().dispatchAsync(ev);
            break;
        }
        case stringHash("03"): {
            //Sat Status
            break;
        }
        case stringHash("04"): {
            //TOD
            break;
        }
    default:
        Logger::instance().debug("GnssReader", "Unknown UBX token: " + split[0]);
    }
}

void GnssReader::handleRmc(const std::string& talker, const std::string& line) {
    //TODO: Add talker handling
    auto split = splitString(line, ',');

    double lat = nmeaPositionToDecimal(split[2], split[3]);
    double lon = nmeaPositionToDecimal(split[4], split[5]);
    double spd = strtod(split[6].c_str(), nullptr);
    double hdg = strtod(split[7].c_str(), nullptr);

    auto ev = std::make_shared<GNSSPositionEvent>();
    ev->setLatitude(lat);
    ev->setLongitude(lon);
    ev->setSpeed(spd);
    ev->setHeading(hdg);
    EventDispatcher::instance().dispatchAsync(ev);
}

void GnssReader::handleTxt(const std::string& line) {
    auto split = splitString(line, ',');
    switch (stringHash(split[2].c_str())) {
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


