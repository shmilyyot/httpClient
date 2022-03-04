#include "Header.h"


Header::Header(const RawData& data){data_ = data;}

const RawData& Header::getRawData() const { return data_; }

bool Header::remove(std::string key)
{
    for (size_t i = 0; i < data_.size(); ++i){
        if(data_[i].first == key){
            data_.erase(data_.begin() + i);
            return true;
        }
    }
    return false;
}

void Header::removeAll(std::string key)
{
    while(remove(key));
}

void Header::add(std::string key,std::string value)
{
    data_.emplace_back(key,value);
}

std::string Header::get(std::string key) const{
    key = string_handler::to_lower_str(key);
    for (int i = 0; i < Header::data_.size(); ++i)
    {
        if( data_[i].first == key)
        {
            return data_[i].second;
        }
    }
    return "";
}