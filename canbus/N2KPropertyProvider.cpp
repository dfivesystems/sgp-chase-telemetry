#include "N2KPropertyProvider.h"
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <boost/json.hpp>
#include "../logging/Logger.h"

namespace json = boost::json;

void N2KPropertyProvider::addPropertyContainer(const N2KContainer& container) {
    n2kContainers_[container.devicePropContainerKey] = container;
}

std::shared_ptr<N2KContainer> N2KPropertyProvider::getPropertyContainer(const std::string& uid) {
    if(n2kContainers_.contains(uid)){
        return std::make_shared<N2KContainer>(n2kContainers_[uid]);
    }
    return nullptr;
}

std::list<N2KProperty> N2KPropertyProvider::findAllProperties() const {
    std::list<N2KProperty> retList;
    for(const auto& val : n2kContainers_ | std::views::values){
        for(auto const& p : val.fields){
            retList.push_back(p);
        }
    }
    return retList;
}

std::shared_ptr<N2KProperty> N2KPropertyProvider::findN2KPropertyByUid(const std::string& uid) const {
    for(const auto& val : n2kContainers_ | std::views::values){
        for(auto const& p : val.fields){
            if(p.uid == uid){
                return std::make_shared<N2KProperty>(p);
            }
        }
    }
    return nullptr;
}

static std::string jsonValueToString(const boost::json::value& v) {
    using boost::json::kind;

    switch (v.kind()) {
    case kind::string:
        return std::string(v.as_string());

    case kind::int64:
        return std::to_string(v.as_int64());

    case kind::uint64:
        return std::to_string(v.as_uint64());

    case kind::double_:
        return std::to_string(v.as_double());

    case kind::bool_:
        return v.as_bool() ? "true" : "false";

    default:
        return {}; // null, array, object → empty
    }
}

void N2KPropertyProvider::loadProperties() {
    Logger::instance().info("N2KPropertyProvider","Loading PGNs");
    std::ifstream file("../n2kpgns.json");
    if (!file.is_open()) {
        Logger::instance().error("N2KPropertyProvider","Error loading PGNs");
        return;
    }

    // Read entire file into a string (Boost.JSON parses from buffers / strings)
    std::ostringstream oss;
    oss << file.rdbuf();
    const std::string text = oss.str();

    json::error_code ec;
    json::value rootVal = json::parse(text, ec);
    if (ec) {
        Logger::instance().error("N2KPropertyProvider","Error parsing PGN file: " + ec.message());
        return;
    }

    if (!rootVal.is_array()) {
        Logger::instance().error("N2KPropertyProvider","Error parsing PGN file, root is not an array");
        return;
    }

    const json::array& root = rootVal.as_array();

    for (const json::value& objVal : root) {
        if (!objVal.is_object()) continue;
        const json::object& obj = objVal.as_object();

        N2KContainer dpc;

        // Note: .if_contains avoids exceptions when key missing
        auto get_str = [&](const char* key) -> std::string {
            if (auto* v = obj.if_contains(key))
                return std::string(v->as_string());
            return {};
        };
        auto get_bool = [&](const char* key) -> bool {
            if (auto* v = obj.if_contains(key); v && v->is_bool())
                return v->as_bool();
            return false;
        };
        auto get_u32 = [&](const char* key) -> unsigned int {
            if (auto* v = obj.if_contains(key); v && v->is_int64())
                return static_cast<unsigned int>(v->as_int64());
            if (auto* v = obj.if_contains(key); v && v->is_uint64())
                return static_cast<unsigned int>(v->as_uint64());
            return 0u;
        };
        auto get_u64 = [&](const char* key) -> std::uint64_t {
            if (auto* v = obj.if_contains(key); v && v->is_uint64())
                return v->as_uint64();
            if (auto* v = obj.if_contains(key); v && v->is_int64() && v->as_int64() >= 0)
                return static_cast<std::uint64_t>(v->as_int64());
            return 0ull;
        };

        dpc.name = get_str("Name");
        if (auto* v = obj.if_contains("dataKey")) {
            dpc.devicePropContainerKey = jsonValueToString(*v);
        }
        dpc.description = get_str("Description");
        dpc.singleFrame = get_bool("SingleFrame");
        dpc.destination = get_bool("Destination");
        dpc.defaultPriority = get_u32("DefaultPriority");
        dpc.defaultUpdateRate = get_u64("DefaultUpdateRate");

        if (auto* fieldsVal = obj.if_contains("Fields"); fieldsVal && fieldsVal->is_array()) {
            const json::array& fields = fieldsVal->as_array();

            for (const json::value& propVal : fields) {
                if (!propVal.is_object()) continue;
                const json::object& propObj = propVal.as_object();

                auto get_prop_str = [&](const char* key) -> std::string {
                    if (auto* v = propObj.if_contains(key); v && v->is_string())
                        return std::string(v->as_string());
                    return {};
                };
                auto get_prop_bool = [&](const char* key) -> bool {
                    if (auto* v = propObj.if_contains(key); v && v->is_bool())
                        return v->as_bool();
                    return false;
                };
                auto get_prop_u32 = [&](const char* key) -> unsigned int {
                    if (auto* v = propObj.if_contains(key); v && v->is_int64())
                        return static_cast<unsigned int>(v->as_int64());
                    if (auto* v = propObj.if_contains(key); v && v->is_uint64())
                        return static_cast<unsigned int>(v->as_uint64());
                    return 0u;
                };
                auto get_prop_i32 = [&](const char* key) -> int {
                    if (auto* v = propObj.if_contains(key); v && v->is_int64())
                        return static_cast<int>(v->as_int64());
                    if (auto* v = propObj.if_contains(key); v && v->is_uint64())
                        return static_cast<int>(v->as_uint64());
                    return 0;
                };
                auto get_prop_double = [&](const char* key) -> double {
                    if (auto* v = propObj.if_contains(key)) {
                        if (v->is_double()) return v->as_double();
                        if (v->is_int64())  return static_cast<double>(v->as_int64());
                        if (v->is_uint64()) return static_cast<double>(v->as_uint64());
                    }
                    return 0.0;
                };

                N2KProperty dp;
                dp.fieldOrder = get_prop_i32("Number");

                unsigned int bytes = get_prop_u32("Bytes");
                unsigned int bits  = get_prop_u32("Bits");
                dp.bitLength = (bytes * 8u) + bits;

                dp.uid = dpc.devicePropContainerKey + "-" + std::to_string(dp.fieldOrder);

                // dictionary: object of string->string
                if (auto* dictVal = propObj.if_contains("dictionary"); dictVal && dictVal->is_object()) {
                    const json::object& dictObj = dictVal->as_object();
                    for (auto const& kv : dictObj) {
                        const std::string key = std::string(kv.key());
                        const json::value& v = kv.value();
                        if (v.is_string()) {
                            dp.dictionary[key] = std::string(v.as_string());
                        } else {
                            // If values aren't strings, preserve something reasonable
                            dp.dictionary[key] = json::serialize(v);
                        }
                    }
                }

                dp.name = get_prop_str("Name");
                dp.alternativeName = get_prop_str("alternativeName");
                dp.activeByDefault = get_prop_bool("activeByDefault");
                dp.includeInSummaryByDefault = get_prop_bool("includeInSummaryByDefault");
                dp.controllable = get_prop_bool("controllable");
                dp.loggingByDefault = get_prop_bool("loggingByDefault");
                dp.dataType = get_prop_str("type");
                dp.minVal = get_prop_double("minVal");
                dp.maxVal = get_prop_double("maxVal");
                dp.multiplier = get_prop_double("multiplier");
                dp.offset = get_prop_double("offset");
                dp.unit = get_prop_str("unit");
                dp.category = get_prop_str("category");
                dp.persistProperty = get_prop_bool("persistProperty");
                dp.instantReport = get_prop_bool("instantReport");

                dpc.fields.push_back(std::move(dp));
            }
        }
        Logger::instance().trace("N2KPropertyProvider", "Adding PGN " + dpc.devicePropContainerKey);
        addPropertyContainer(dpc);
    }
}
