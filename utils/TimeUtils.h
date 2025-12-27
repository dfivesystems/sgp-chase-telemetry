#ifndef TIMEUTILS_H
#define TIMEUTILS_H
#include <boost/asio/detail/chrono.hpp>

using namespace boost::asio::chrono;

inline unsigned long systemTimeMillis() {
	return duration_cast<milliseconds>(
			   system_clock::now().time_since_epoch()
		   ).count();
}

#endif //TIMEUTILS_H
