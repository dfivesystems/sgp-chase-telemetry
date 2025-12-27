#ifndef N2KPROPERTYPROVIDER_H
#define N2KPROPERTYPROVIDER_H
#include <fstream>
#include <map>
#include <memory>
#include "N2KProperty.h"

class N2KPropertyProvider {
public:
    static N2KPropertyProvider& instance(){
        static N2KPropertyProvider instance;
        return instance;
    }
    void addPropertyContainer(const N2KContainer&);
    std::shared_ptr<N2KContainer> getPropertyContainer(const std::string& uid);
    std::list<N2KProperty> findAllProperties() const;
    std::shared_ptr<N2KProperty> findN2KPropertyByUid(const std::string& uid) const;
    void loadProperties();

    N2KPropertyProvider(const N2KPropertyProvider& other) = delete;
    N2KPropertyProvider& operator= (const N2KPropertyProvider &other) = delete;
    N2KPropertyProvider(const N2KPropertyProvider&& other) = delete;
    N2KPropertyProvider& operator= (const N2KPropertyProvider&& other) = delete;
private:
    N2KPropertyProvider()= default;
    std::map<std::string, N2KContainer> n2kContainers_ = {};
};


#endif //N2KPROPERTYPROVIDER_H
