#include <iostream>
#include <thread>
#include <boost/asio/io_context.hpp>

#include "config/ConfigProvider.h"
#include "event/EventDispatcher.h"
#include "gnss/GnssReader.h"
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

    boost::asio::io_context ioCtx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ioCtx.get_executor()};
    std::thread ioThread = std::thread([&]() {
        ioCtx.run();
    });
    GnssReader reader(ioCtx, ConfigProvider::instance().serialPort());

    ioThread.join();
    return 0;
}
