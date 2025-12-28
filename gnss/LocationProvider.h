#ifndef LOCATIONPROVIDER_H
#define LOCATIONPROVIDER_H
#include <boost/asio/steady_timer.hpp>

#include "../event/EventDispatcher.h"
//TODO: Expand for multi-constellation analysis

struct LocationSourceEntry {
	GNSSSource source;
	unsigned long long lastSeen;
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
	//TODO: Add sats in view
};

class LocationProvider final: public EventListener {
public:
	explicit LocationProvider(boost::asio::io_context& ctx);
	~LocationProvider();

	void notifyMessage(std::shared_ptr<Event> ev) override;

private:
	boost::asio::steady_timer timer_;

	LocationSourceEntry* fixIsValid();
	void timeout(const boost::system::error_code& ec);
	LocationSourceEntry* getOrCreateSource(GNSSSource source);
	void handleGnssPositionEvent(const std::shared_ptr<GNSSPositionEvent>& ev);
	void handleGnssSatellitesEvent(const std::shared_ptr<Event>& ev);
	void handleGnssTodEvent(const std::shared_ptr<Event>& ev);
	std::vector<LocationSourceEntry> locationSources_;
};



#endif //LOCATIONPROVIDER_H
