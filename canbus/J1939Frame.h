#ifndef J1939FRAME_H
#define J1939FRAME_H

#include <iterator>
#include <linux/can.h>

class J1939Frame {
public:
    explicit J1939Frame(can_frame &frame) {
        const unsigned int canId = frame.can_id;
        priority_ = canId >> 26 & 0b111;
        dataPage_ = canId >> 24 & 0b1; //Feel the force Luke
        pduFormat_ = canId >> 16 & 0xFF;
        dstAddress_ = canId >> 8 & 0xFF;
        srcAddress_ = canId & 0xFF;
        frameLength_ = frame.can_dlc;
        if (pduFormat_ < 240) {
            pgn_ = (canId & 0x01FF0000) >> 8;
        } else {
            pgn_ = (canId & 0x01FFFF00) >> 8;
        }
        std::ranges::copy(frame.data, std::begin(data_));
    }

    unsigned char srcAddress() const { return srcAddress_; }

    unsigned char dstAddress() const { return dstAddress_; }

    unsigned char priority() const { return priority_; }

    unsigned char dataPage() const { return dataPage_; }

    unsigned char pduFormat() const { return pduFormat_; }

    unsigned char frameLength() const { return frameLength_; }

    unsigned int pgn() const { return pgn_; }

    unsigned char* data() { return data_; }

private:
    unsigned char priority_;
    unsigned char srcAddress_;
    unsigned char dstAddress_;//PDU Specific
    unsigned char pduFormat_;
    unsigned char dataPage_;
    unsigned char frameLength_;
    unsigned int pgn_;
    unsigned char data_[8];
};
#endif //J1939FRAME_H
