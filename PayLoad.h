#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include "String_handler.h"

namespace payload
{
    struct Binary
    {
        std::vector<char> data_;
        std::string file_name_;
        std::string type_;
        Binary& load_file(std::string path, std::string type);
    };

    class FormData
    {
        std::vector<std::pair<std::string, std::vector<char>>> data_;
        std::string boundary_;
        public:
            FormData() { boundary_ = "----httpClientBoundary" + std::to_string(long(this)); }
            ~FormData() {}
            void append(std::string name, Binary value);
            void append(std::string name, std::string value);
            bool remove(std::string key);
            const std::vector<std::pair<std::string, std::vector<char>>>& getData() const;
            std::string getContentType();
            std::vector<char> serialize();
    };
};