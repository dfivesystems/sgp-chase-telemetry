#ifndef LINKCLIENT___STRINGUTILS_H
#define LINKCLIENT___STRINGUTILS_H

#include <algorithm>
#include <sstream>

constexpr unsigned int stringHash(const char *str, int offset = 0) {
    return !str[offset] ? 5381 : (stringHash(str, offset+1)*33) ^ str[offset];
}

static std::string getStringFromBuffer(const char* buffer, size_t startPos, size_t len){
    std::stringstream ss;
    for(int i = 0; i < len; i++) {
        char c = *(buffer + startPos + i);
        if (c >= 0x20 || c == 0x0A || c == 0x0D) {
            ss << c;
        }
    }
    return ss.str();
}

static std::string getHexStringFromBuffer(char* buffer, size_t startPos, size_t len){
    std::stringstream ss;
    for(int i = 0; i < len; i++){
        ss << std::hex << buffer+startPos+i;
    }
    return ss.str();
}

static std::string toLower(const std::string& inStr){
    std::string outStr;
    std::ranges::transform(inStr,
                           outStr.begin(),[](unsigned char c){ return std::tolower(c); });
    return outStr;
}

static std::vector<std::string> splitString(const std::string &str, const char delimiter){
    std::vector<std::string> retArr;
    auto start = 0U;
    auto end = str.find(delimiter);
    while (end != std::string::npos){
        retArr.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    retArr.emplace_back(str.substr(start, str.length() - 1));
    return retArr;
}
#endif //LINKCLIENT___STRINGUTILS_H
