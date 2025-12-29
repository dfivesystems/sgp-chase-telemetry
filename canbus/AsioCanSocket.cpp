#include "AsioCanSocket.h"

#include "N2KPropertyProvider.h"
#include "../logging/Logger.h"

//TODO: Implement instance naming
//TODO: Test against real devices

AsioCanSocket::AsioCanSocket(const std::string& interfaceName, boost::asio::io_context& ioCtx): stream_(ioCtx){
    Logger::instance().info("AsioCanSocket", "Opening CAN socket " + interfaceName);
    EventDispatcher::instance().subscribe(POSITION, this);
    sockaddr_can addr{};
    ifreq ifr{};

    sockFd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, interfaceName.c_str());
    ioctl(sockFd_, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(sockFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        Logger::instance().error("AsioCanSocket", "Failed to bind to socket");
        return;
    }
    stream_.assign(sockFd_);
    readOperation();
    genericISORequest(60928, 255);
}

void AsioCanSocket::readOperation(){
    stream_.async_read_some( boost::asio::buffer(&recFrame_, sizeof(recFrame_)),
                            [&](const boost::system::error_code &ec,
                                std::size_t) {
                                if(!ec) {
                                    J1939Frame msg(recFrame_);
                                    handleMessage(msg);
                                } else {
                                    Logger::instance().error("AsioCanSocket", "CAN Receive error - " + ec.message());
                                }
                                this->readOperation();
                            });
}

std::array<uint8_t, 8> AsioCanSocket::calculateLocalName() const{
    std::array<uint8_t, 8> dataBuf = {};
    const unsigned int uniqueNumber = (UID_ & 0x1FFFFF) | (MFR_CODE &0x7ff << 21);
    //TODO: Check order
    dataBuf[0] = uniqueNumber << 24;
    dataBuf[1] = uniqueNumber << 16;
    dataBuf[2] = uniqueNumber << 8;
    dataBuf[3] = uniqueNumber << 0;
    dataBuf[4] = 0x00;
    dataBuf[5] = DEV_FUNC;
    dataBuf[6] = ((DEV_CLASS &0x7f)<<1);
    dataBuf[7] = (0x80 | (0x04&0x7)<< 4 | 0x01 & 0x0f);
    return dataBuf;
}

void AsioCanSocket::addressClaim(){
    write(60928, 255, 3, calculateLocalName().data(), 8);
}

void AsioCanSocket::genericISORequest(const uint32_t pgn, const uint8_t addr) {
    uint8_t dataBuf[8] = {};
    dataBuf[0] = pgn >> 0;
    dataBuf[1] = pgn >> 8;
    dataBuf[2] = pgn >> 16;
    write(59904, addr, 3, dataBuf, 8);
}

unsigned long AsioCanSocket::nameFromBytes(const uint8_t* name){
    return ((name[0] & 0xFFL)  << 56) |
           ((name[1] & 0xFFL) << 48) |
           ((name[2] & 0xFFL) << 40) |
           ((name[3] & 0xFFL) << 32) |
           ((name[4] & 0xFFL) << 24) |
           ((name[5] & 0xFFL) << 16) |
           ((name[6] & 0xFFL) << 8)  |
           ( name[7] & 0xFFL);
}

void AsioCanSocket::processAddressClaim(CanMessage &msg) {
    if(msg.source() == localAddress_){
//            If someone has the same address as us, calculate our name vs theirs
        Logger::instance().info("AsioCanSocket", "Address claim requested");
        const unsigned long localNameInt = nameFromBytes(calculateLocalName().data());
        if(const auto remoteName = msg.data(); remoteName.size() < 8){
            return;
        }
        if(const unsigned long remoteNameInt = nameFromBytes(msg.data().data()); localNameInt > remoteNameInt){
            //We lose the contention
            Logger::instance().error("AsioCanSocket", "Address claim lost");
            localAddress_ +=1;
            addressClaim();
        }
    }
    if(const CanDevice* device = getOrCreateDevice(msg.source()); !device->detailsInitialised) {
        genericISORequest(126996, msg.source());
    }
}

void AsioCanSocket::sendProductDetails(){
    uint8_t buffer[134];
    buffer[0] = static_cast<uint8_t>(DB_VERSION << 8);
    buffer[1] = static_cast<uint8_t>(DB_VERSION);
    buffer[2] = static_cast<uint8_t>(PRODUCT_CODE << 8);
    buffer[3] = static_cast<uint8_t>(PRODUCT_CODE);
    char strBuf[32];
    //Bytes 4 - > 35, MODEL ID
    memset(strBuf, 0xFF, 32);
    strncpy(strBuf, MODEL_ID, strlen(MODEL_ID) < 32 ? strlen(MODEL_ID) : 32);
    memcpy(buffer + 4, strBuf, 32);
    //Bytes 36 -> 67, SOFTWARE VERSION ID
    memset(strBuf, 0xFF, 32);
    strncpy(strBuf, SOFTWARE_VERSION, strlen(SOFTWARE_VERSION) < 32 ? strlen(SOFTWARE_VERSION) : 32);
    memcpy(buffer + 36, strBuf, 32);
    //Bytes 68 -> 99, MODEL VERSION
    memset(strBuf, 0xFF, 32);
    strncpy(strBuf, MODEL_VERSION, strlen(MODEL_VERSION) < 32 ? strlen(MODEL_VERSION) : 32);
    memcpy(buffer + 68, strBuf, 32);
    //bytes 100 -> 131, Device Serial
    memset(strBuf, 0xFF, 32);
    strncpy(strBuf, SERIAL_NO, strlen(SERIAL_NO) < 32 ? strlen(SERIAL_NO) : 32);
    memcpy(buffer + 100, strBuf, 32);
    //Byte 132 Cert Level
    buffer[132] = CERT_LEVEL;
    //Byte 133 Load Equivalency
    buffer[133] = LOAD_EQUIVALENCY;
    write(126996, 0, 6, buffer, 134);
}

void AsioCanSocket::handleMessage(J1939Frame& frame){
    unsigned char length = frame.frameLength();
    auto dpc = N2KPropertyProvider::instance().getPropertyContainer(std::to_string(frame.pgn()));
    if(nullptr == dpc){
        Logger::instance().warn("AsioCanSocket", "Property container not found for " + std::to_string(frame.pgn()));
        return;
    }
    if(dpc->singleFrame){
        CanMessage msg(dpc.get(), length, 0x00, frame.srcAddress(), frame.dstAddress());
        msg.addToMessage(0x00, frame);
        Logger::instance().trace("AsioCanSocket", "Received single frame message for " + std::to_string(frame.pgn()) + " ("+ dpc->name+") from address " + std::to_string(frame.srcAddress()));
        msg.populateFieldData();
        handleCompleteMessage(msg);
    } else {
        unsigned char frameNo = frame.data()[0] & 0b00011111;
        unsigned char sequence = frame.data()[0] & 0b11100000;
        std::string msgRef = std::to_string(frame.srcAddress()) + "-" + std::to_string(frame.pgn()) + "-" + std::to_string(sequence);
        if(frameNo == 0){
            length = frame.data()[1];
            CanMessage msg(dpc.get(), length, sequence, frame.srcAddress(), frame.dstAddress());
            msg.addToMessage(frameNo, frame);
            messageStore_[msgRef] = msg;
        } else {
            if(!messageStore_.contains(msgRef)){
                Logger::instance().warn("AsioCanSocket", "No message found for " + msgRef);
                return;
            }
            CanMessage msg = messageStore_[msgRef];
            msg.addToMessage(frameNo, frame);
            if(msg.isComplete()){
                messageStore_.erase(msgRef);
                Logger::instance().trace("AsioCanSocket", "Received complete message for " + msgRef);
                msg.populateFieldData();
                handleCompleteMessage(msg);
            }
        }
    }
}

CanDevice* AsioCanSocket::getOrCreateDevice(uint8_t addr){
    if(!deviceStore_.contains(addr)) {
        Logger::instance().trace("AsioCanSocket", "Creating new device reference at addr " + std::to_string(addr));
        auto device = CanDevice();
        device.address = addr;
        deviceStore_.emplace(addr, device);
    }
    return &(deviceStore_.at(addr));
}

void AsioCanSocket::handleCompleteMessage(CanMessage &msg) {
    if(msg.pgn() == "60928"){
        Logger::instance().debug("AsioCanSocket", "Address claim received");
        processAddressClaim(msg);
        return;
    }
    CanDevice* device = getOrCreateDevice(msg.source());
    switch(stringHash(msg.pgn().c_str())) {
        case stringHash("59904"): {
            Logger::instance().debug("AsioCanSocket", "ISO Request");
            if (msg.destination() == localAddress_ || msg.destination() == 255) {
                const int pgn = (msg.data()[2] & 0xFF) << 16 | (msg.data()[1] & 0xFF) << 8 |
                          (msg.data()[0] & 0xFF);
                Logger::instance().debug("AsioCanSocket", "Requested PGN: " + std::to_string(pgn));
                if (pgn == 60928) {
                    addressClaim();
                } else if (pgn == 126996) {
                    sendProductDetails();
                }
            }
            break;
        }
        case stringHash("60928"): {
            processAddressClaim(msg);
            break;
        }
        case stringHash("126993"): {
            if (!device->detailsInitialised) {
                genericISORequest(126996, msg.source());
            }
            break;
        }
        case stringHash("126996"): {
            std::cout << "Device Details" << std::endl;
            break;
        }
        case stringHash("126983"): {
            //TODO: When we see some alerts - generate this properly
            std::cout << "Alert" << std::endl;
            break;
        }

        default:{
            const auto ev = std::make_shared<NMEAPropertyEvent>(device->uid);
            for(const auto& [key, val] : msg.stringMap()){
                //Only send update if values have changed
                if(device->updateValue(key, msg.instance(), val)){
                    ev->addValue(key, msg.instance(), val);
                }
            }
            if(!ev->values().empty()){
                EventDispatcher::instance().dispatchAsync(ev);
            }
            break;
        }
    }
}

void AsioCanSocket::write(const uint32_t pgn, const uint8_t remoteAddress, const uint8_t priority, const uint8_t* data, const uint8_t dataSize){
    if(const auto dpc = N2KPropertyProvider::instance().getPropertyContainer(std::to_string(pgn)); nullptr != dpc && !dpc->singleFrame){
        const int frameCount = static_cast<int>(std::ceil(dataSize / 7.0));
        uint8_t packetId = 0b00100000;
        const uint8_t payloadLen = dataSize;
        uint8_t arr[8] = {};
        arr[0] = packetId;
        arr[1] = payloadLen;
        arr[2] = data[0];
        arr[3] = data[1];
        arr[4] = data[2];
        arr[5] = data[3];
        arr[6] = data[4];
        arr[7] = data[5];
        const can_frame frame = generateFrame(pgn, remoteAddress, priority, arr);
        //Write initial frame
        writeRawFrame(frame);
        int cnt = 6;
        for(int fc = 1; fc < frameCount; fc++){
            packetId += 1;
            uint8_t fup[8] = {};
            fup[0] = packetId;
            for(int i = 1; i < 8; i++){
                if(cnt < payloadLen){
                    fup[i] = data[cnt];
                } else {
                    fup[i] = 0xFF;
                }
                cnt++;
            }
            const can_frame fFrame = generateFrame(pgn, remoteAddress, priority, fup);
            writeRawFrame(fFrame);
        }
    } else {
        const can_frame frame = generateFrame(pgn, remoteAddress, priority, data);
        writeRawFrame(frame);
    }
}

void AsioCanSocket::handlePositionEvent(const std::shared_ptr<PositionEvent>& ev) {
    static uint8_t sid = 0;
    const int32_t lat = ev->latitude * 1e7;
    const int32_t lon = ev->longitude * 1e7;
    const uint16_t hdg = ev->heading * 0.0174533 * 1e4;
    const uint16_t spd = ev->speed * 100;

    uint8_t posRapid[8];
    uint8_t cogSog[8];

    posRapid[0] = lat & 0xFF;
    posRapid[1] = (lat >> 8) & 0xFF;
    posRapid[2] = (lat >> 16) & 0xFF;
    posRapid[3] = (lat >> 24) & 0xFF;
    posRapid[4] = lon & 0xFF;
    posRapid[5] = (lon >> 8) & 0xFF;
    posRapid[6] = (lon >> 16) & 0xFF;
    posRapid[7] = (lon >> 24) & 0xFF;


    cogSog[0] = sid;
    cogSog[1] = (1 & 0x03u) | 0xFCu;
    cogSog[2] = hdg & 0xFF;
    cogSog[3] = (hdg >> 8) & 0xFF;
    cogSog[4] = spd & 0xFF;
    cogSog[5] = (spd >> 8) & 0xFF;
    cogSog[6] = 0xFF;
    cogSog[7] = 0xFF;

    write(129025, 255, 6, posRapid, 8);
    write(129026, 255, 6, cogSog, 8);
    sid++;
}

void AsioCanSocket::writeRawFrame(can_frame frame){
    stream_.async_write_some(boost::asio::buffer(&frame, sizeof(frame)),
                            [&](const boost::system::error_code &ec,
                                    const std::size_t bytes_transferred){asyncWriteHandler(ec, bytes_transferred);});
}

void AsioCanSocket::asyncWriteHandler(const boost::system::error_code &ec, const std::size_t transferred) {
    if(!ec){
        Logger::instance().trace("AsioCanSocket", std::to_string(transferred) + " bytes written to socket successfully");
    } else {
        Logger::instance().error("AsioCanSocket", "CAN error while writing to socket - " + ec.message());
    }
}

uint32_t AsioCanSocket::generateHeader(const uint32_t pgn, const uint8_t remoteAddress, const uint8_t priority) const{
    uint32_t can_id = 0;
    const uint8_t pf = (pgn >> 8) & 0xFF;
    uint8_t ps;
    if (pf < 240) {
        ps = remoteAddress;
    } else {
        ps = pgn & 0xFF;
    }

    can_id |= (priority & 0x7) << 26;
    can_id |= (pgn & 0x1FFFF) << 8;
    can_id &= ~(0xFF << 8);
    can_id |= static_cast<uint32_t>(ps) << 8;
    can_id |= localAddress_;

    return can_id;
}

can_frame AsioCanSocket::generateFrame(const uint32_t pgn, const uint8_t remoteAddress, const uint8_t priority, const uint8_t* data) const{
    can_frame frame = {};
    frame.can_id = generateHeader(pgn, remoteAddress, priority);
    frame.can_id |= CAN_EFF_FLAG;
    frame.can_dlc = 8;
    memcpy(frame.data, data, 8);
    return frame;
}

void AsioCanSocket::notifyMessage(const std::shared_ptr<Event> ev) {
    if (ev->eventType() == POSITION) {
        handlePositionEvent(std::dynamic_pointer_cast<PositionEvent>(ev));
    }
}
