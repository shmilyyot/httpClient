#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "String_handler.h"
#include "Header.h"

class Response
{
    friend class Request;
    std::vector<char> body_;    //响应体
    bool is_chunked_ = false;   //是否分块
    int content_length_ = 0;    //正文内容长度
    std::vector<char> source_;  //源响应报文
    std::string protocol_version_;
    std::string code_;
    std::string status_;
    Header header_;     //响应头
    size_t body__pos_ = -1;    //报文头起始位置

public:
    Response(){};
    ~Response(){};
    Response(std::vector<char> source);
    const std::vector<char> &getBody();
    const std::string getCode();
    const std::string getStatus();
    const int getContentLength();
    const Header& getHeader() const;

private:
    void parse_header();
    void parse_body();
    bool can_parse_header();
};