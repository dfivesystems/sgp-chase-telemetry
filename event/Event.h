#ifndef EVENT_H
#define EVENT_H
#include <cmath>
#include <iostream>
#include <vector>

enum EventType{
    NONE = 0,
    NMEA_PROPERTY,
    NMEA_BUS,
    GNSS_POSITION,
    GNSS_SATELLITES,
    GNSS_TOD,
    RTK_CORRECTION,
    COURSE_UPDATE,
    ASSET_POSITION,
    RAG_STATUS,
    COMMITTEE_MESSAGE
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

enum RAGStatus {
    RED = 0,
    AMBER,
    GREEN
};

struct Event {
    virtual ~Event() = default;
    EventType eventType() const {return eventType_;};
protected:
    EventType eventType_ = NONE;
};
///
/// Property records represent an individual value instance for a NMEA property
///
struct PropertyRecord {
    PropertyRecord(const std::string& propertyUid, const std::string& instance, const std::string& value);
    std::string propertyUid;
    std::string instance;
    std::string value;
};
///
///A NMEA Property even is used for processing the PGNs coming of the bus. It contains all the processed values
/// as presented by the bus, with dictionaries applied where necessary
///
struct NMEAPropertyEvent final: Event {
    explicit NMEAPropertyEvent(const std::string& deviceUid);
    std::string deviceUid;
    std::vector<PropertyRecord> values();
    void addValue(const std::string& propertyUid, const std::string& instance, const std::string& value);
private:
    std::vector<PropertyRecord> values_;
};
///
/// A NMEA PGN Event is used for sending values to the N2K Bus. The pgn should be set to the required PGN number
/// and the fields vector should contain the full list of fields used in the PGN
///
struct NMEAPGNEvent final: Event {
    NMEAPGNEvent();
    std::string pgn;
    std::vector<std::string> values;
};

///
/// GNSS Position events are used to send the raw position information from either the N183 or N2k inputs to the
/// location provider for processing. The location provider then sends these to the relevant outputs (N2K, MDSS, influx)
///
struct GNSSPositionEvent final : Event {
    GNSSPositionEvent();
    double latitude = NAN;
    double longitude = NAN;
    double altitude = NAN;
    double hAccuracy = NAN;
    double vAccuracy = NAN;
    double speed = NAN;
    double heading = NAN;
    double vVelocity = NAN;
    double correctionAge = NAN;
    double hdop = NAN;
    GNSSSatelliteConstellation constellation = UNKNOWN;
};

///
/// GNSS Satellite records represent an individual satellite instance in a GSV or SatsInView Message
struct GNSSSatelliteRecord {
    int satelliteId = 0;
    int elevation = 0;
    int azimuth = 0;
    int snr = 0;
};

struct GNSSSatellitesEvent final : Event {
    GNSSSatellitesEvent();
    unsigned int satsInView = 0;
    std::vector<GNSSSatelliteRecord> satellites;
    GNSSSatelliteConstellation constellation = GNSSSatelliteConstellation::GPS;
};

struct GNSSTodEvent final: Event {

};

struct RTKCorrectionEvent final: Event {

};

struct CourseUpdateEvent final: Event {

};

struct AssetPositionEvent final: Event {

};

struct RAGStatusEvent final: Event {

};

struct CommitteeMessageEvent final: Event {

};

#endif //EVENT_H
