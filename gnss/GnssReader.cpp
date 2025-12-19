#include "GnssReader.h"
#include "../utils/StringUtils.h"

#include <iostream>
#include <boost/asio/read_until.hpp>

#include "../utils/NMEAUtils.h"

GnssReader::GnssReader(boost::asio::io_context& ioCtx): serialPort(ioCtx) {
    boost::system::error_code ec;
    serialPort.open("/dev/ttyACM0", ec);
    if(ec) {
        std::cerr << "Error opening GNSS serial port: " << ec.message() << std::endl;
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
        std::cerr << "Error receiving data form serial port " << ec.message() << std::endl;
    }
    readOperation();
}

void GnssReader::handlePacket(const std::string& line) {
    size_t splitPos = line.find_first_of(',');
    if(splitPos == std::string::npos || splitPos == 0) {
        // std::cout << "Invalid line format, cannot parse token" << std::endl;
        // std::cout << "LINE: " << line << std::endl;
        return;
    }
    std::string token = line.substr(0, splitPos);
    if(token.at(0) != '$'){
        std::cout << "Invalid token start char: " << token << std::endl;
        std::cout << "LINE: " << line << std::endl;
        return;
    }
    if(line.at(line.size()-3) != '*') {
        std::cout << "No checksum in " << token << " message" << std::endl;
        std::cout << "LINE: " << line << std::endl;
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
        std::cout << "Unhandled token " << token << std::endl;
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
        std::cout << "Unhandled UBX Message type " << split[0] << std::endl;
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


