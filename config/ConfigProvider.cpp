#include "ConfigProvider.h"

#include <boost/json.hpp>
#include <fstream>
#include <sstream>
#include "../logging/Logger.h"

using namespace boost::json;

ConfigProvider& ConfigProvider::instance() {
    static ConfigProvider instance;
    return instance;
}

void ConfigProvider::loadConfig(const std::string& filePath) {
    Logger::instance().info("ConfigProvider",  "Loading Config File");
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Logger::instance().critical("ConfigProvider", "Unable to load config");
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    value root = parse(buffer.str());
    object& obj = root.as_object();

    assetName_ = value_to<std::string>(obj.at("assetName"));
    riedelBoatNumber_ = value_to<int>(obj.at("riedelBoatNumber"));
    serialPort_ = value_to<std::string>(obj.at("serialPort"));
    influxAddress_ = value_to<std::string>(obj.at("influxAddress"));
    mdssAddress_ = value_to<std::string>(obj.at("mdssAddress"));
    plotterAddress_ = value_to<std::string>(obj.at("plotterAddress"));

    nmeaPgnFilter_.clear();
    for (const auto& v : obj.at("nmeaPgnFilter").as_array()) {
        nmeaPgnFilter_.push_back(value_to<int>(v));
    }

    nmeaInstanceMapping_.clear();
    const object& pgnMap = obj.at("nmeaInstanceMapping").as_object();

    for (const auto& [pgnKey, instancesValue] : pgnMap) {
        int pgn = std::stoi(std::string(pgnKey));

        const object& instancesObj = instancesValue.as_object();
        auto& instanceMap = nmeaInstanceMapping_[pgn];

        for (const auto& [instanceKey, nameValue] : instancesObj) {
            int instance = std::stoi(std::string(instanceKey));
            instanceMap[instance] = value_to<std::string>(nameValue);
        }
    }
    Logger::instance().info("ConfigProvider", "Config Loaded Successfully");
}

const std::string& ConfigProvider::assetName() const {
    return assetName_;
}

int ConfigProvider::riedelBoatNumber() const {
    return riedelBoatNumber_;
}

const std::string& ConfigProvider::serialPort() const {
    return serialPort_;
}

const std::string& ConfigProvider::influxAddress() const {
    return influxAddress_;
}

const std::string& ConfigProvider::mdssAddress() const {
    return mdssAddress_;
}

const std::string& ConfigProvider::plotterAddress() const {
    return plotterAddress_;
}

const std::vector<int>& ConfigProvider::nmeaPgnFilter() const {
    return nmeaPgnFilter_;
}

const std::unordered_map<int,
    std::unordered_map<int, std::string>>&
ConfigProvider::nmeaInstanceMapping() const {
    return nmeaInstanceMapping_;
}

std::string ConfigProvider::lookupInstanceName(int pgn, int instance) const {
    auto pgnIt = nmeaInstanceMapping_.find(pgn);
    if (pgnIt == nmeaInstanceMapping_.end()) {
        return {};
    }

    auto instIt = pgnIt->second.find(instance);
    if (instIt == pgnIt->second.end()) {
        return {};
    }

    return instIt->second;
}

