#ifndef GNSSREADER_H
#define GNSSREADER_H
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include "../event/Event.h"

enum N183GNSSQualityIndicator {
    INVALID = 0,
    GNSS,
    DGPS,
    NA,
    RTK_FIXED,
    RTK_FLOAT,
    INS_DR
};

class GnssReader {
public:
    GnssReader(boost::asio::io_context &ioCtx, const std::string &port);

private:
    boost::asio::serial_port serialPort_;

    void readOperation();
    void readHandler(const boost::system::error_code &ec, std::size_t length);
    static void handlePacket(const std::string& line);
    static bool validateChecksum(const std::string& line);

    std::array<char, 1024> dataBuf_ = {};
    boost::asio::streambuf buffer_;

    static void handleUbx(const std::string& sentence);
    static void handleRmc(const std::string& talker, const std::string& sentence);
    static void handleGga(const std::string& talker, const std::string& sentence);
    static void handleGsa(const std::string& talker, const std::string& sentence);
    static void handleGsv(const std::string& talker, const std::string& sentence);
    static void handleGll(const std::string& talker, const std::string& sentence);
    static void handleVtg(const std::string& talker, const std::string& sentence);
    static void handleTxt(const std::string &line);

    static GNSSSatelliteConstellation constellationFromTalker(const std::string &talker);
};



#endif //GNSSREADER_H
