#ifndef ASIOCANSOCKET_H
#define ASIOCANSOCKET_H

#include <iostream>
#include <unistd.h>
#include <boost/asio.hpp>
#include <linux/can.h>
#include <sys/types.h>
#include "CanDevice.h"
#include "CanMessage.h"
#include "J1939Frame.h"
#include "../event/Event.h"
#include "../event/EventDispatcher.h"

static constexpr uint16_t MFR_CODE = 1100;
static constexpr uint8_t DEV_CLASS = 0x19;
static constexpr uint8_t DEV_FUNC =  0x82;
static constexpr uint16_t PRODUCT_CODE = 11222;
static constexpr uint16_t DB_VERSION = 2101;
static constexpr char MODEL_ID[] = "Riedel SmartChase";
static constexpr char SOFTWARE_VERSION[] = "Build 0.1";
static constexpr char MODEL_VERSION[] = "Riedel SmartChase";
static constexpr char SERIAL_NO[] = "42";
static constexpr uint8_t CERT_LEVEL = 2;
static constexpr uint8_t LOAD_EQUIVALENCY = 3;

class AsioCanSocket: public std::enable_shared_from_this<AsioCanSocket> {
public:
    AsioCanSocket(const std::string& interfaceName, boost::asio::io_context& ioCtx);
    void writeRawFrame(can_frame frame);
    void readOperation();
    uint32_t generateHeader(uint32_t pgn, uint8_t remoteAddress, uint8_t priority) const;
    can_frame generateFrame(uint32_t pgn, uint8_t remoteAddress, uint8_t priority, const uint8_t* data) const;
private:
    int sockFd_;
    boost::asio::posix::basic_stream_descriptor<> stream_;
    can_frame recFrame_ = {};
    uint8_t localAddress_ = 42;
    std::map<std::string, CanMessage> messageStore_;
    std::map<uint8_t, CanDevice> deviceStore_;

    static void asyncWriteHandler(const boost::system::error_code& ec, std::size_t transferred);
    void handleMessage(J1939Frame& frame);
    CanDevice* getOrCreateDevice(uint8_t addr);
    void handleCompleteMessage(CanMessage& msg);
    void processAddressClaim(CanMessage& msg);
    void genericISORequest(uint32_t pgn, uint8_t addr);
    void sendProductDetails();
    void addressClaim();
    void write(uint32_t pgn, uint8_t remoteAddress, uint8_t priority, const uint8_t *data, uint8_t dataSize);

    std::array<uint8_t, 8> calculateLocalName() const;

    static unsigned long nameFromBytes(const uint8_t *name);

    unsigned int UID_ = stringHash(SERIAL_NO);
};


#endif //ASIOCANSOCKET_H
