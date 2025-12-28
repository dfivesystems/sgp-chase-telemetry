#include "Event.h"

PropertyRecord::PropertyRecord(const std::string &propertyUid, const std::string &instance, const std::string &value) {
    this->propertyUid = propertyUid;
    this->instance = instance;
    this->value = value;
}

NMEAPropertyEvent::NMEAPropertyEvent(const std::string& deviceUid) {
    eventType_ = EventType::NMEA_PROPERTY;
    this->deviceUid = deviceUid;
}

void NMEAPropertyEvent::addValue(const std::string& propertyUid, const std::string& instance, const std::string& value) {
    values_.emplace_back(propertyUid, instance, value);
}

std::vector<PropertyRecord> NMEAPropertyEvent::values(){
    return values_;
}

GNSSPositionEvent::GNSSPositionEvent() {
    eventType_ = EventType::GNSS_POSITION;
}

GNSSSatellitesEvent::GNSSSatellitesEvent() {
    eventType_ = EventType::GNSS_SATELLITES;
}
