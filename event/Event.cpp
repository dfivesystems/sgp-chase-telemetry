#include "Event.h"

PropertyRecord::PropertyRecord() = default;
PropertyRecord::PropertyRecord(const std::string &propertyUid, const std::string &instance, const std::string &value) {
    propertyUid_ = propertyUid;
    instance_ = instance;
    value_ = value;
}

std::string PropertyRecord::propertyUid() {
    return propertyUid_;
}

std::string PropertyRecord::instance() {
    return instance_;
}

std::string PropertyRecord::value() {
    return value_;
}

NMEAPropertyEvent::NMEAPropertyEvent(const std::string& deviceUid) {
    eventType_ = EventType::NMEA_PROPERTY;
    deviceUid_ = deviceUid;
}

std::string NMEAPropertyEvent::deviceUid() {
    return deviceUid_;
}

void NMEAPropertyEvent::addValue(const std::string& propertyUid, const std::string& instance, const std::string& value) {
    values_.emplace_back(propertyUid, instance, value);
}

std::vector<PropertyRecord> NMEAPropertyEvent::values(){
    return values_;
}

NMEABusEvent::NMEABusEvent(const std::string& deviceUid, const std::string& propertyUid,
                                                 const std::string& propertyInstance, const std::string& value) {
    eventType_ = EventType::NMEA_BUS;
    deviceUid_ = deviceUid;
    propertyUid_ = propertyUid;
    propertyInstance_ = propertyInstance;
    value_ = value;
}

std::string NMEABusEvent::deviceUid() {
    return deviceUid_;
}

std::string NMEABusEvent::propertyUid() {
    return propertyUid_;
}

std::string NMEABusEvent::propertyInstance() {
    return propertyInstance_;
}

std::string NMEABusEvent::propertyValue() {
    return value_;
}

GNSSPositionEvent::GNSSPositionEvent() {
    eventType_ = EventType::GNSS_POSITION;
}

double GNSSPositionEvent::latitude() const {
    return latitude_;
}

void GNSSPositionEvent::setLatitude(double latitude) {
    latitude_ = latitude;
}

double GNSSPositionEvent::longitude() const {
    return longitude_;
}

void GNSSPositionEvent::setLongitude(double longitude) {
    longitude_ = longitude;
}

double GNSSPositionEvent::altitude() const {
    return altitude_;
}

void GNSSPositionEvent::setAltitude(double altitude) {
    altitude_ = altitude;
}

double GNSSPositionEvent::hAccuracy() const {
    return hAccuracy_;
}

void GNSSPositionEvent::setHAccuracy(double h_accuracy) {
    hAccuracy_ = h_accuracy;
}

double GNSSPositionEvent::vAccuracy() const {
    return vAccuracy_;
}

void GNSSPositionEvent::setVAccuracy(double v_accuracy) {
    vAccuracy_ = v_accuracy;
}

double GNSSPositionEvent::speed() const {
    return speed_;
}

void GNSSPositionEvent::setSpeed(double speed) {
    speed_ = speed;
}

double GNSSPositionEvent::heading() const {
    return heading_;
}

void GNSSPositionEvent::setHeading(double heading) {
    heading_ = heading;
}

double GNSSPositionEvent::vVelocity() const {
    return vVelocity_;
}

void GNSSPositionEvent::setVVelocity(double v_velocity) {
    vVelocity_ = v_velocity;
}

double GNSSPositionEvent::correctionAge() const {
    return correctionAge_;
}

void GNSSPositionEvent::setCorrectionAge(double correction_age) {
    correctionAge_ = correction_age;
}

double GNSSPositionEvent::hdop() const {
    return hdop_;
}

void GNSSPositionEvent::setHdop(double hdop) {
    hdop_ = hdop;
}

GNSSSatellitesEvent::GNSSSatellitesEvent() {
    eventType_ = EventType::GNSS_SATELLITES;
}

void GNSSSatellitesEvent::setSatsInView(const int satsInView) {
    satsInView_ = satsInView;
}

int GNSSSatellitesEvent::satsInView() const {
    return satsInView_;
}

void GNSSSatellitesEvent::addSatelliteRecord(GNSSSatelliteRecord record) {
    satellites_.push_back(record);
}

std::vector<GNSSSatelliteRecord> GNSSSatellitesEvent::satellites() const {
    return satellites_;
}

void GNSSSatellitesEvent::setConstellation(GNSSSatelliteConstellations constellation) {
    constellation_ = constellation;
}

GNSSSatelliteConstellations GNSSSatellitesEvent::constellation() const {
    return constellation_;
}
