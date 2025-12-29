#include "LocationProvider.h"
LocationProvider::LocationProvider(boost::asio::io_context& ctx): timer_(ctx) {
	EventDispatcher::instance().subscribe(GNSS_POSITION, this);
	EventDispatcher::instance().subscribe(GNSS_SATELLITES, this);
	EventDispatcher::instance().subscribe(GNSS_TOD, this);
	timer_.expires_after(boost::asio::chrono::milliseconds(100));
	timer_.async_wait([&](const boost::system::error_code& ec) {
		timeout(ec);
	});
}

LocationProvider::~LocationProvider() {
	EventDispatcher::instance().unsubscribe(GNSS_POSITION, this);
	EventDispatcher::instance().unsubscribe(GNSS_SATELLITES, this);
	EventDispatcher::instance().unsubscribe(GNSS_TOD, this);
}

void LocationProvider::notifyMessage(const std::shared_ptr<Event> ev) {
	switch (ev->eventType()) {
		case GNSS_POSITION:
			handleGnssPositionEvent(std::dynamic_pointer_cast<GNSSPositionEvent>(ev));
			break;
		case GNSS_SATELLITES:
			handleGnssSatellitesEvent(ev);
			break;
		case GNSS_TOD:
			handleGnssTodEvent(ev);
			break;
		default:
			//noop
			break;
	}
}

LocationSourceEntry* LocationProvider::fixIsValid() {
	for (auto & locationSource : locationSources_) {
		//TODO: Make this more sophisticated
		if (!std::isnan(locationSource.latitude) && !std::isnan(locationSource.longitude)) {
			return &locationSource;
		}
	}
	return nullptr;
}

void LocationProvider::timeout(const boost::system::error_code& ec) {
	//TODO: Error handling
	//Work out if we have a valid GPS fix and send a packet to N2K, influx and MDSS if we do
	if (const LocationSourceEntry* src = fixIsValid()) {
		const auto ev = std::make_shared<PositionEvent>();
		ev->latitude = src->latitude;
		ev->longitude = src->longitude;
		ev->altitude = src->altitude;
		ev->speed = src->speed;
		ev->heading = src->heading;
		EventDispatcher::instance().dispatchAsync(ev);
	}
	timer_.expires_from_now(boost::asio::chrono::milliseconds(100));
	timer_.async_wait([&](const boost::system::error_code& ec) {
		this->timeout(ec);
	});
}

LocationSourceEntry* LocationProvider::getOrCreateSource(const GNSSSource source) {
	for (auto& locationSource : locationSources_) {
		if (locationSource.source == source) {
			return &locationSource;
		}
	}
	LocationSourceEntry entry;
	entry.source = source;
	locationSources_.push_back(entry);
	return &locationSources_.back();
}

void LocationProvider::handleGnssPositionEvent(const std::shared_ptr<GNSSPositionEvent>& ev) {
	LocationSourceEntry* src = getOrCreateSource(ev->source);
	src->latitude = ev->latitude;
	src->longitude = ev->longitude;
	src->altitude = ev->altitude;
	if (!std::isnan(ev->speed)) {
		src->speed = ev->speed;
	}
	if (!std::isnan(ev->heading)) {
		src->heading = ev->heading;
	}
}

void LocationProvider::handleGnssSatellitesEvent(const std::shared_ptr<Event>& ev) {

}

void LocationProvider::handleGnssTodEvent(const std::shared_ptr<Event>& ev) {

}
