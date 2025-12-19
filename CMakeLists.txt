cmake_minimum_required(VERSION 3.30)
project(sgp_chase_telemetry)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

add_executable(sgp_chase_telemetry main.cpp
        utils/NMEAUtils.h
        gnss/GnssReader.cpp
        gnss/GnssReader.h
        logging/Logger.cpp
        logging/Logger.h)

if(TARGET Boost::headers)
    target_link_libraries(sgp_chase_telemetry PRIVATE Boost::headers)
elseif(TARGET Boost::boost) # FindBoost module
    target_link_libraries(sgp_chase_telemetry PRIVATE Boost::boost)
else()
    target_include_directories(sgp_chase_telemetry PRIVATE ${Boost_INCLUDE_DIRS})
endif()
target_link_libraries(sgp_chase_telemetry PRIVATE Threads::Threads)
