#include <iostream>
#include <thread>
#include <boost/asio/io_context.hpp>

int main() {
    std::cout << "Riedel Chase Telemetry Service";
    std::cout << "Starting Chase Telemetry Service" << std::endl;
    std::cout << "Reticulating Splines" << std::endl;
    std::cout << "Acquiring Targets" << std::endl;
    std::cout << "Locking tractor beam" << std::endl;
    std::cout << "Energizing warp core" << std::endl;
    std::cout << "Brewing tea" << std::endl;
    boost::asio::io_context ioCtx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_{ioCtx.get_executor()};
    std::thread ioThread = std::thread([&]() {
        ioCtx.run();
    });

    ioThread.join();
    return 0;
}