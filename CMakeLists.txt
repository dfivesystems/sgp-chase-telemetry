cmake_minimum_required(VERSION 3.20)
project(sgp_chase_telemetry)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS json)

add_executable(sgp_chase_telemetry main.cpp
        utils/NMEAUtils.h
        gnss/GnssReader.cpp
        gnss/GnssReader.h
        logging/Logger.cpp
        logging/Logger.h
        config/ConfigProvider.cpp
        config/ConfigProvider.h
        event/Event.cpp
        event/Event.h
        event/EventDispatcher.cpp
        event/EventDispatcher.h
        canbus/AsioCanSocket.cpp
        canbus/AsioCanSocket.h
        canbus/CanDevice.h
        canbus/CanMessage.h
        canbus/J1939Frame.h
        canbus/N2KProperty.h
        canbus/N2KPropertyProvider.cpp
        canbus/N2KPropertyProvider.h
        utils/TimeUtils.h
)

if(TARGET Boost::headers)
    target_link_libraries(sgp_chase_telemetry PRIVATE Boost::headers Boost::json)
elseif(TARGET Boost::boost) # FindBoost module
    target_link_libraries(sgp_chase_telemetry PRIVATE Boost::boost Boost::json)
else()
    target_include_directories(sgp_chase_telemetry PRIVATE ${Boost_INCLUDE_DIRS})
endif()
target_link_libraries(sgp_chase_telemetry PRIVATE Threads::Threads)
