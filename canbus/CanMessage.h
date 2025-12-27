#ifndef CANMESSAGE_H
#define CANMESSAGE_H

#include <map>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "J1939Frame.h"
#include "N2KProperty.h"
#include "../utils/StringUtils.h"

class CanMessage {
public:
    CanMessage() = default;

    explicit CanMessage(N2KContainer *property,
                        unsigned char length, unsigned char sequence, uint8_t source, uint8_t destination);

    void addToMessage(unsigned char frameNumber, J1939Frame &frame);

    [[nodiscard]] bool isComplete() const;

    void populateFieldData();

    std::map<std::string, std::string> stringMap();
    std::string pgn();
    [[nodiscard]] uint8_t source() const;
    [[nodiscard]] uint8_t destination() const;
    std::vector<uint8_t>data();
    std::string instance();

private:
    bool singleFrame_ = false;
    std::string pgn_ = "";
    uint8_t source_ = 0;
    uint8_t destination_ = 0;
    uint8_t length_ = 0;
    uint8_t magic_ = 0;
    uint8_t nextExpectedFrame_ = 0;
    uint8_t sequence_ = 0;
    std::string instance_ = "";
    std::vector<uint8_t> messageBytes_ = {};
    std::map<std::string, std::string> stringMap_ = {};
    N2KContainer *propertyContainer_ = nullptr;
};

inline CanMessage::CanMessage(N2KContainer *property, const unsigned char length, const unsigned char sequence,
    const uint8_t source, const uint8_t destination) {
    singleFrame_ = property->singleFrame;
    this->sequence_ = sequence;
    pgn_ = property->devicePropContainerKey;
    this->length_ = length;
    nextExpectedFrame_ = 0;
    magic_ = 0;
    instance_ = "0";
    propertyContainer_ = property;
    this->source_ = source;
    this->destination_ = destination;
}

inline void CanMessage::addToMessage(unsigned char frameNumber, J1939Frame &frame) {
    if (singleFrame_) {
        magic_ += 8;
        messageBytes_.assign(frame.data(), frame.data() + 8);
    } else {
        if (nextExpectedFrame_ != frameNumber) {
            //throw exception
            return;
        }
        if (frameNumber == 0) {
            for (int i = 2; i < 8; i++) {
                messageBytes_.push_back(frame.data()[i]);
            }
            magic_ += 6;
        } else {
            for (int i = 1; i < 8; i++) {
                if (i + magic_ < messageBytes_.size()) {
                    messageBytes_.push_back(frame.data()[i]);
                }
            }
            magic_ += 7;
        }
        nextExpectedFrame_++;
    }
}

inline bool CanMessage::isComplete() const {
    return magic_ >= length_;
}

inline void CanMessage::populateFieldData() {
    boost::dynamic_bitset<> bitset(length_ * 8);
    for (int i = 0; i < length_; ++i) {
        uint8_t cur = messageBytes_[i];
        int offset = i * CHAR_BIT;
        for (int bit = 0; bit < CHAR_BIT; ++bit) {
            bitset[offset] = cur & 1;
            ++offset;   // Move to next bit in b
            cur >>= 1;  // Move to next bit in array
        }
    }
    unsigned int cursor = 0;
    for (const auto &field: propertyContainer_->fields) {
        std::string value = "Not Available";
        switch (stringHash(field.dataType.c_str())) {
            case stringHash("string"):
            case stringHash("String"):
                //get string
                break;
            case stringHash("uint8"): {
                uint8_t val = 0;
                for (uint8_t i = 0; i < 8; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 8;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = val + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = val * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("uint16"): {
                uint16_t val = 0;
                for (uint16_t i = 0; i < 16; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 16;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = val + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = val * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("uint32"): {
                uint32_t val = 0;
                for (uint32_t i = 0; i < 32; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 32;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = val + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = val * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("uint64"): {
                uint64_t val = 0;
                for (uint16_t i = 0; i < 64; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 64;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    long double dVal = static_cast<long double>(val) + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    long double dVal = val * static_cast<long double>(field.multiplier) + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("int8"): {
                int8_t val = 0;
                for (int8_t i = 0; i < 8; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 8;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = val + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = val * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("int16"): {
                int16_t val = 0;
                for (int16_t i = 0; i < 16; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 16;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = val + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = val * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("int32"): {
                int32_t val = 0;
                for (int32_t i = 0; i < 32; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 32;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = val + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = val * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("int64"): {
                int64_t val = 0;
                for (uint64_t i = 0; i < 64; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 64;
                if (field.multiplier == 1 || field.multiplier == 0) {
                    long double dVal = static_cast<long double>(val) + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    long double dVal = static_cast<long double>(val) * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("float32"): {
                uint32_t val = 0;
                for (uint32_t i = 0; i < 32; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 32;
                auto fVal = static_cast<float>(val);
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = fVal + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = fVal * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("float64"): {
                uint64_t val = 0;
                for (uint64_t i = 0; i < 64; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += 64;
                auto fVal = static_cast<double>(val);
                if (field.multiplier == 1 || field.multiplier == 0) {
                    double dVal = fVal + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                } else {
                    double dVal = fVal * field.multiplier + field.offset;
                    if (dVal < field.minVal || dVal > field.maxVal) {
                        value = "N/A";
                    } else {
                        value = std::to_string(dVal);
                    }
                }
                break;
            }
            case stringHash("bitfield"): {
                uint64_t val = 0;
                for (uint32_t i = 0; i < field.bitLength; i++) {
                    if (bitset[i + cursor]) {
                        val |= 1 << i;
                    }
                };
                cursor += field.bitLength;
                value = std::to_string(val);
                break;
            }
        }
        if (field.name.find("Instance") != std::string::npos) {
            instance_ = value;
        }
        stringMap_[field.uid] = value;
    }
}

inline std::map<std::string, std::string> CanMessage::stringMap() {
    return stringMap_;
}

inline std::string CanMessage::pgn(){
    return pgn_;
}

inline uint8_t CanMessage::source() const {
    return source_;
}

inline uint8_t CanMessage::destination() const {
    return destination_;
}

inline std::vector<uint8_t> CanMessage::data(){
    return messageBytes_;
}

inline std::string CanMessage::instance() {
    return instance_;
}

#endif //CANMESSAGE_H
