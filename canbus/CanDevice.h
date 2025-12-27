#ifndef CANDEVICE_H
#define CANDEVICE_H

#include <map>
#include <string>
#include "../utils/TimeUtils.h"

struct CanValueRecord {
    unsigned long long lastUpdate;
    std::string value;
};

struct CanDevice {
    bool updateValue(const std::string& propertyUid, const std:: string& instance, const std::string& value){
        const std::string key = propertyUid + instance;
        if(value.empty()){
            return false;
        }
        if(!values.contains(key)){
            values[key] = {.lastUpdate = systemTimeMillis(), .value = value};
            return true;
        }
        if(value == values[key].value && systemTimeMillis() - values[key].lastUpdate < 5000){
            return false;
        }
        values[key] = {.lastUpdate = systemTimeMillis(), .value = value};
        return true;
    }

    std::string uid = "";
    int address = -1;
    bool detailsInitialised = false;
    std::map<std::string, CanValueRecord> values;
};


#endif //CANDEVICE_H
