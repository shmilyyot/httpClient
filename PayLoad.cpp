#include "PayLoad.h"

payload::Binary& payload::Binary::load_file(std::string path, std::string type)
{
    std::fstream file(path, std::ios_base::in);
    file_name_ = path;
    type_ = type;
    const int size = 1024;
    std::array<char, size> buf;
    while(file.good())
    {
        file.read(buf.data(), size);
        data_.insert(data_.end(), buf.begin(), buf.begin() + file.gcount());
    }
    return *this;
}

void payload::FormData::append(std::string name, Binary value)
{
    std::vector<char> res;
    std::string filename = value.file_name_.size() ? "; filename=\"" + value.file_name_ + "\"" : "";
    std::string str = "--" + boundary_ + string_handler::crlf;
    str += "Content-Disposition: form-data; name=\"" + name + "\"";
    str += filename;
    str += string_handler::crlf;
    if( value.type_ != "")
    {
        str += "Content-type: " + value.type_;
        str += string_handler::crlf;
    }
    str += string_handler::crlf;
    string_handler::str_to_charvector(res, str);
    res.insert(res.end(), value.data_.begin(), value.data_.end());
    string_handler::str_to_charvector(res, string_handler::crlf);
    data_.emplace_back(name, res);
}

void payload::FormData::append(std::string name, std::string value)
{
    std::vector<char> res;
    std::string str = "--" + boundary_ + string_handler::crlf;
    str += "Content-Disposition: form-data; name=\"" + name + "\"" + string_handler::crlf;
    str += string_handler::crlf;
    str += value;
    str += string_handler::crlf;
    string_handler::str_to_charvector(res, str);
    data_.emplace_back(name, res);
}

bool payload::FormData::remove(std::string key)
{
    for (size_t i = 0; i < data_.size(); +i)
    {
        if(data_[i].first == key)
        {
            data_.erase(data_.begin() + i);
            return true;
        }
    }
    return false;
}

const std::vector<std::pair<std::string, std::vector<char>>>& payload::FormData::getData() const
{
    return data_; 
}

std::string payload::FormData::getContentType() 
{ 
    return "multipart/form-data; boundary=" + boundary_; 
}

std::vector<char> payload::FormData::serialize()
{
    std::vector<char> res;
    for(auto& data : data_)
    {
        res.insert(res.end(), data.second.begin(), data.second.end());
    }
    res.push_back('-');
    res.push_back('-');
    string_handler::str_to_charvector(res, boundary_);
    res.push_back('-');
    res.push_back('-');
    return res;
}