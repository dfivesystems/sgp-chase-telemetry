#include "GnssReader.h"
#include "../utils/StringUtils.h"

#include <boost/asio/read_until.hpp>

#include "../utils/NMEAUtils.h"
#include "../logging/Logger.h"

GnssReader::GnssReader(boost::asio::io_context& ioCtx): serialPort(ioCtx) {
    boost::system::error_code ec;
    serialPort.open("/dev/ttyACM0", ec);
    if(ec) {
        Logger::instance().error("GnssReader", "Error opening GNSS serial port: " + ec.message());
        return;
    }
    readOperation();
}

void GnssReader::readOperation() {
    boost::asio::async_read_until(serialPort, buffer, "\r\n", [this](const boost::system::error_code ec, std::size_t length) { readHandler(ec, length); });
}

void GnssReader::readHandler(const boost::system::error_code &ec, std::size_t length) {
    if(!ec) {
        std::string line;
        std::istream(&buffer) >> line;
        handlePacket(line);
    } else {
        Logger::instance().error("GnssReader", "Error receiving data from serial port: " + ec.message());
    }
    readOperation();
}

void GnssReader::handlePacket(const std::string& line) {
    size_t splitPos = line.find_first_of(',');
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
        Logger::instance().warn("GnssReader", "No checksum in " + token + "message");
        Logger::instance().debug("GnssReader", "LINE: " + line);
        return;
    }
    //TODO: Validate checksum
    //TODO: add missing handlers
    std::string stripped = line.substr(splitPos+1, line.size()-3);
    switch(stringHash(token.c_str())) {
        case stringHash("$PUBX"): {
            handleUbx(stripped);
            break;
        }
        case stringHash("$GNRMC"): {
            handleRmc(stripped);
            break;
        }
        case stringHash("$GNGGA"): {
            break;
        }
        case stringHash("$GNGSA"): {
            break;
        }
        case stringHash("$GPGSV"): {
            break;
        }
        case stringHash("$GLGSV"): {
            break;
        }
        case stringHash("$GAGSV"): {
            break;
        }
        case stringHash("$GBGSV"): {
            break;
        }
        case stringHash("$GQGSV"): {
            break;
        }
        case stringHash("$GNGLL"): {
            break;
        }
        case stringHash("$GNVTG"): {
            break;
        }
        default:
        Logger::instance().debug("GnssReader", "Unknown token: " + token);
    }
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

            //TODO: Send location message
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

void GnssReader::handleRmc(const std::string& line) {
    auto split = splitString(line, ',');

    double lat = nmeaPositionToDecimal(split[2], split[3]);
    double lon = nmeaPositionToDecimal(split[4], split[5]);
    double spd = strtod(split[6].c_str(), nullptr);
    double hdg = strtod(split[7].c_str(), nullptr);

    //TODO: Send location message
}


