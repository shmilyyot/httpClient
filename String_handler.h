#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

namespace string_handler{

    //windows下
    static std::string crlf = "\r\n";
    static std::string crlf_crlf = "\r\n\r\n";
    static std::string default_trim_symbol = " \n\r";
    //linux下
    static std::string lf = "\n";
    static std::string lflf = "\n\n";

    static void str_to_charvector(std::vector<char>& cv,std::string& s)
    {
        cv.insert(cv.end(), s.begin(), s.end());
    }

    static std::string to_upper_str(std::string &s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    static std::string to_lower_str(std::string &s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    static bool hasLetter(std::string &s)
    {
        for(char &c:s)
        {
            if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            {
                return true;
            }
        }
        return false;
    }

    static bool include_symbol(std::string &symbol, char &c)
    {
        for (auto &i : symbol)
        {
            if (i == c)
            {
                return true;
            }
        }
        return false;
    }

    static std::string charvector_to_str(std::vector<char> &source_)
    {
        return source_.size() == 0 ? "" : std::string(source_.data(), 0, source_.size());
    }

    static std::vector<int> find_all_start_pos(std::string &s, std::string &delimiter, int num = -1)
    {
        std::vector<int> ans;
        int pos = s.find(delimiter);
        int delimiter_offset = delimiter.length() == 0 ? 1 : delimiter.length();
        while(pos != std::string::npos && ans.size() != num)
        {
            ans.push_back(pos);
            pos = s.find(delimiter, pos + delimiter_offset);
        }
        return ans;
    }

    //清除首尾的无用字符
    static std::string trim(std::string &s,std::string delimiter = default_trim_symbol)
    {
        int len = s.length();
        int left = 0;
        while(left < len && include_symbol(delimiter, s[left]))
        {
            ++left;
        }
        if(left >= len)
        {
            return s;
        }
        int right = len - 1;
        while (right > 0 && include_symbol(delimiter, s[right]))
        {
            --right;
        }
        return s.substr(left, right - left + 1);
    }

    static std::vector<std::string> split(std::string &s, std::string delimiter, int num = -1, bool skip_empty = true)
    {
        std::vector<std::string> split_data;
        auto start_pos = find_all_start_pos(s, delimiter, num);
        if(start_pos.size() == 0)
        {
            return std::vector<std::string>({s});
        }
        auto push_str = [&](std::string subs)
        {
            if(subs.length() != 0 || !skip_empty)
            {
                split_data.push_back(subs);
            }
        };
        for (int i = 0; i <= start_pos.size(); ++i)
        {
            if(split_data.size() == num && start_pos.size() > num && num != -1)
            {
                push_str(s.substr(start_pos[split_data.size()] + delimiter.size()));
                break;
            }
            if(i == 0)
            {
                push_str(s.substr(0, start_pos[0]));
            }else if( i != start_pos.size())
            {
                int left = start_pos[i - 1] + delimiter.length();
                int right = start_pos[i] - left;
                push_str(s.substr(left, right));
            }
            else
            {
                push_str(s.substr(start_pos[start_pos.size()-1] + delimiter.length()));
            }
        }
        return split_data;
    }
}