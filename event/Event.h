#ifndef EVENT_H
#define EVENT_H
#include <iostream>
#include <vector>
#include <cmath>

enum EventType{
    NONE = 0,
    NMEA_PROPERTY,
    NMEA_BUS,
    GNSS_POSITION,
    GNSS_SATELLITES,
    GNSS_TOD,
    RTK_CORRECTION,
    COURSE_UPDATE,
    ASSET_POSITION
};

enum GNSSSatelliteConstellation {
    UNKNOWN = 0,
    GPS,
    GLONASS,
    BEIDOU,
    GALILEO,
    QZSS,
    NAVIC,
    INS,
    COMBINED
};

class Event {
public:
    virtual ~Event() = default;
    EventType eventType() const {return eventType_;};
protected:
    EventType eventType_ = NONE;
};

class PropertyRecord {
public:
    PropertyRecord();
    PropertyRecord(const std::string& propertyUid, const std::string& instance, const std::string& value);

    std::string propertyUid();
    std::string instance();
    std::string value();
private:
    std::string propertyUid_;
    std::string instance_;
    std::string value_;
};

class NMEAPropertyEvent: public Event {
public:
    explicit NMEAPropertyEvent(const std::string& deviceUid);
    std::string deviceUid();
    std::vector<PropertyRecord> values();
    void addValue(const std::string& propertyUid, const std::string& instance, const std::string& value);

private:
    std::string deviceUid_;
    std::vector<PropertyRecord> values_;
};

class NMEABusEvent: public Event {
public:
    NMEABusEvent(const std::string& deviceUid, const std::string& propertyUid,
                        const std::string& propertyInstance, const std::string& value);
    std::string deviceUid();
    std::string propertyUid();
    std::string propertyInstance();
    std::string propertyValue();

private:
    std::string deviceUid_;
    std::string propertyUid_;
    std::string propertyInstance_;
    std::string value_;
};

class GNSSPositionEvent: public Event {
public:
    GNSSPositionEvent();

    [[nodiscard]] double latitude() const;
    void setLatitude(double latitude);
    [[nodiscard]] double longitude() const;
    void setLongitude(double longitude);
    [[nodiscard]] double altitude() const;
    void setAltitude(double altitude);
    [[nodiscard]] double hAccuracy() const;
    void setHAccuracy(double h_accuracy);
    [[nodiscard]] double vAccuracy() const;
    void setVAccuracy(double v_accuracy);
    [[nodiscard]] double speed() const;
    void setSpeed(double speed);
    [[nodiscard]] double heading() const;
    void setHeading(double heading);
    [[nodiscard]] double vVelocity() const;
    void setVVelocity(double v_velocity);
    [[nodiscard]] double correctionAge() const;
    void setCorrectionAge(double correction_age);
    [[nodiscard]] double hdop() const;
    void setHdop(double hdop);
    [[nodiscard]] GNSSSatelliteConstellation constellation() const;
    void setConstellation(GNSSSatelliteConstellation constellation);

private:
    double latitude_ = NAN;
    double longitude_ = NAN;
    double altitude_ = NAN;
    double hAccuracy_ = NAN;
    double vAccuracy_ = NAN;
    double speed_ = NAN;
    double heading_ = NAN;
    double vVelocity_ = NAN;
    double correctionAge_ = NAN;
    double hdop_ = NAN;
    GNSSSatelliteConstellation constellation_ = UNKNOWN;
};

struct GNSSSatelliteRecord {
    int satelliteId = 0;
    int elevation = 0;
    int azimuth = 0;
    int snr = 0;
};

class GNSSSatellitesEvent: public Event {
public:
    GNSSSatellitesEvent();

    void setSatsInView(const int satsInView);
    [[nodiscard]] int satsInView() const;
    void addSatelliteRecord(GNSSSatelliteRecord record);
    [[nodiscard]] std::vector<GNSSSatelliteRecord> satellites() const;
    void setConstellation(GNSSSatelliteConstellation constellation);
    [[nodiscard]] GNSSSatelliteConstellation constellation() const;

private:
    unsigned int satsInView_ = 0;
    std::vector<GNSSSatelliteRecord> satellites_;
    GNSSSatelliteConstellation constellation_ = GNSSSatelliteConstellation::GPS;
};

class GNSSTodEvent: public Event {
public:
private:
};

class RTKCorrectionEvent: public Event {
public:
private:
};

class CourseUpdateEvent: public Event {
public:
private:
};

class AssetPositionEvent: public Event {
public:
private:
};

#endif //EVENT_H
