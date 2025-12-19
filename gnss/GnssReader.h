#ifndef GNSSREADER_H
#define GNSSREADER_H
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>

class GnssReader {
public:
    explicit GnssReader(boost::asio::io_context &ioCtx);

private:
    boost::asio::serial_port serialPort;
    void readOperation();
    void readHandler(const boost::system::error_code &ec, std::size_t length);
    void handlePacket(const std::string& line);
    std::array<char, 1024> dataBuf = {};
    boost::asio::streambuf buffer;

    void handleUbx(const std::string& string);
    void handleRmc(const std::string &line);
};



#endif //GNSSREADER_H
