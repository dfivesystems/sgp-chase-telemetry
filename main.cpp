#include <iostream>
#include <thread>
#include <boost/asio/io_context.hpp>

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
    boost::asio::io_context ioCtx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ioCtx.get_executor()};
    std::thread ioThread = std::thread([&]() {
        ioCtx.run();
    });
    GnssReader reader(ioCtx);

    ioThread.join();
    return 0;
}
