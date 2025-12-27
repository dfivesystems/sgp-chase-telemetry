#ifndef N2KPROPERTY_H
#define N2KPROPERTY_H
#include <list>
#include <map>
#include <string>

struct N2KProperty {
    std::string name;
    std::string alternativeName;
    bool activeByDefault;
    bool includeInSummaryByDefault;
    unsigned int fieldOrder;

    std::string dataType;
    double minVal;
    double maxVal;
    double multiplier;
    double offset;
    std::string unit;

    std::map<std::string, std::string> dictionary;

    bool persistProperty;
    bool instantReport;
    bool controllable;
    bool loggingByDefault;
    bool optional;
    std::string uid;
    std::string category;
    unsigned int bitLength;
};

struct N2KContainer {
    std::string name;
    std::string devicePropContainerKey;
    std::string description;
    bool singleFrame;
    bool destination;
    unsigned char defaultPriority;
    unsigned char defaultUpdateRate;
    std::list<N2KProperty> fields;
    std::list<N2KProperty> repeatingFields;
};
#endif //N2KPROPERTY_H
