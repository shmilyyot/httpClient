#include "Response.h"
#include "Error.h"

using namespace string_handler;

const std::vector<char> &Response::getBody()
{
    return body_;
}

const std::string Response::getCode()
{
    return code_;
}

const std::string Response::getStatus()
{
    return status_;
}

const int Response::getContentLength()
{
    return content_length_;
}

const Header& Response::getHeader() const {
     return header_; 
}

Response::Response(std::vector<char> source) : source_(source)
{
    parse_header();
    parse_body();
}

bool Response::can_parse_header()
{
    std::string buf_str = charvector_to_str(source_);
    //lflf连续两个换行符，相当于找到上一行换行和空行，跳过这两个符号就是正文了
    //顺便设置相应报文正文开始的位置
    if (body__pos_ == -1 && buf_str.find(crlf_crlf) != -1)
    {
        body__pos_ = buf_str.find(crlf_crlf) + 4;
    }
    bool check = (body__pos_ != std::string::npos);
    return body__pos_ != std::string::npos;
}

void Response::parse_header()
{
    std::string buf_str = charvector_to_str(source_);
    std::string head_str = buf_str.substr(0, body__pos_);
    auto header_data = split(head_str, crlf);
    if(header_data.size() == 0)
    {
        return;
    }
    //请求行
    auto request_line = split(header_data[0], " ", 2);
    check(request_line.size() == 3, "响应报文解析错误\n" + buf_str);
    protocol_version_ = trim(request_line[0]);
    code_ = trim(request_line[1]);
    status_ = trim(request_line[2]);
    //将请求行删掉
    header_data.erase(header_data.begin());
    //请求头部
    for(std::string data : header_data)
    {
        std::cout << data << std::endl;
        auto content_line = split(data, ":", 1);
        if(content_line.size() == 2)
        {
            std::string k = trim(content_line[0]);
            std::string v = trim(content_line[1]);
            header_.add(to_lower_str(k), v);
        }
    }
    std::string content_len = header_.get("content-length");
    content_length_ = content_len != "" ? stoi(content_len) : content_length_;
    is_chunked_ = header_.get("Transfer-Encoding") == "chunked";
    //将除去头部之后多于下来的请求数据存入body
    body_.insert(body_.end(), source_.begin() + body__pos_, source_.end());
}

void Response::parse_body()
{
    const auto& body = body_;
    if(body.size() == 0 || !is_chunked_)
    {
        return;
    }
    std::cout << "----------当前报文采用chunk分块编码----------" << std::endl;
    std::vector<char> pure_source_char;
    int crlf_pos = 0;
    auto get_next_crlf = [&](int leap)
    {
        for (int i = crlf_pos + leap; i < (body.size() - 1); ++i)
        {
            if(body[i] == '\r' && body[i+1] == '\n')
            {
                crlf_pos = i;
                return i;
            }
        }
        return -1;
    };
    int left = -2;
    int right = get_next_crlf(0);
    int chunk_count = 0;
    while (left != -1 && right != -1)
    {
        ++chunk_count;
        std::string chunk_num_str = std::string(body.begin() + 2 + left, body.begin() + right);
        auto chunk_num = stoi(chunk_num_str, nullptr, 16);
        std::cout << "当前第 "<< chunk_count << " 块的大小是：" << chunk_num << std::endl;
        if (chunk_num == 0)
        {
            break;
        }
        auto chunk_start = body.begin() + right + 2;
        pure_source_char.insert(pure_source_char.end(), chunk_start, chunk_start + chunk_num);
        left = get_next_crlf(chunk_num + 2);
        right = get_next_crlf(1);
    }
    body_ = pure_source_char;
    content_length_ = pure_source_char.size();
}