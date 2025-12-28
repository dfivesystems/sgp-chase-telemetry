#include <iostream>
#include <thread>
#include <boost/asio/io_context.hpp>

#include "canbus/AsioCanSocket.h"
#include "canbus/N2KPropertyProvider.h"
#include "config/ConfigProvider.h"
#include "event/EventDispatcher.h"
#include "gnss/GnssReader.h"
#include "gnss/LocationProvider.h"
#include "logging/Logger.h"

int main() {
    std::cout << "Riedel Chase Telemetry Service\n";
    Logger::instance().info("main", "Starting Chase Telemetry Service");
    Logger::instance().info("main",  "Reticulating Splines");
    Logger::instance().info("main",  "Acquiring Targets");
    Logger::instance().info("main",  "Locking tractor beam");
    Logger::instance().info("main",  "Energizing warp core");
    Logger::instance().info("main",  "Brewing tea");
    //TODO: Parameterize with arguments
    ConfigProvider::instance().loadConfig("../config_template.json");
    //Dummy call to start threads
    EventDispatcher::instance();
    N2KPropertyProvider::instance().loadProperties();

    boost::asio::io_context ioCtx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ioCtx.get_executor()};
    std::thread ioThread = std::thread([&]() {
        ioCtx.run();
    });
    LocationProvider locationProvider(ioCtx);
    GnssReader reader(ioCtx, ConfigProvider::instance().serialPort());
    AsioCanSocket canSkt("vcan0", ioCtx);

    ioThread.join();
    return 0;
}
