#ifndef GNSSREADER_H
#define GNSSREADER_H
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>

class GnssReader {
public:
    GnssReader(boost::asio::io_context &ioCtx, const std::string &port);

private:
    boost::asio::serial_port serialPort_;

    void readOperation();
    void readHandler(const boost::system::error_code &ec, std::size_t length);
    void handlePacket(const std::string& line);
    std::array<char, 1024> dataBuf_ = {};
    boost::asio::streambuf buffer_;

    void handleUbx(const std::string& string);
    void handleRmc(const std::string &line);
};



#endif //GNSSREADER_H
