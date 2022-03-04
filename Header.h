#pragma once
#include <string>
#include <vector>
#include "String_handler.h"

using RawData = typename std::vector<std::pair<std::string,std::string>>;

class Header{
    public:
        Header(){};
        Header(const RawData &data);
        ~Header(){};
        const RawData& getRawData() const;
        bool remove(std::string key);
        void removeAll(std::string key);
        void add(std::string key, std::string value);
        std::string get(std::string key) const;
    private:
        RawData data_;
};