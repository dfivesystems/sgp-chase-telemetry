#ifndef CONFIGPROVIDER_H
#define CONFIGPROVIDER_H

#include <boost/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class ConfigProvider {
public:
    static ConfigProvider& instance();

    void loadConfig(const std::string& filePath);

    const std::string& assetName() const;
    int riedelBoatNumber() const;
    const std::string& serialPort() const;
    const std::string& influxAddress() const;
    const std::string& mdssAddress() const;
    const std::string& plotterAddress() const;
    const std::vector<int>& nmeaPgnFilter() const;

    const std::unordered_map<int,
        std::unordered_map<int, std::string>>& nmeaInstanceMapping() const;

    std::string lookupInstanceName(int pgn, int instance) const;

private:
    ConfigProvider() = default;

    std::string assetName_;
    int riedelBoatNumber_{0};
    std::string serialPort_;
    std::string influxAddress_;
    std::string mdssAddress_;
    std::string plotterAddress_;
    std::vector<int> nmeaPgnFilter_;

    std::unordered_map<int,
        std::unordered_map<int, std::string>> nmeaInstanceMapping_;
};

#endif //CONFIGPROVIDER_H
