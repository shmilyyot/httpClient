#include "Request.h"

Request& Request::setMethod(Method method)
{
    switch (method)
    {
        case Method::GET:
            method_ = "GET";
            break;
        case Method::POST:
            method_ = "POST";
            break;
    }
    return *this;
}

Request& Request::setUrl(std::string url)
{
    url_ = url;
    return *this;
}

Request& Request::setBody(std::string body)
{
    request_body_.clear();
    string_handler::str_to_charvector(request_body_, body);
    return *this;
}

Request& Request::setBody(payload::Binary body)
{
    request_body_.clear();
    request_body_.insert(request_body_.end(), body.data_.begin(), body.data_.end());
    if(body.type_ != "")
    {
        header_.removeAll("Content-Type");
        header_.add("Content-Type", body.type_);
    }
    return *this;
}

Request& Request::setBody(payload::FormData body)
{
    request_body_ = body.serialize();
    header_.removeAll("Content-Type");
    header_.add("Content-Type", body.getContentType());
    return *this;
}

std::vector<char>& Request::getBody()
{
    return request_body_;
}

Request& Request::setHeader(Header header)
{
    header_ = header;
    return *this;
}

Request& Request::setHeader(std::string key,std::string value)
{
    header_.add(key, value);
    return *this;
}

Header& Request::getHeader(){
    return header_;
}

Request& Request::setProxy(HttpProxy proxy)
{
    enable_proxy_ = true;
    proxy_ = proxy;
    return *this;
}

Response Request::sendRequest(Method method, std::string url)
{
    setMethod(method);
    return sendRequest(url);
}

Response Request::sendRequest() { return sendRequest(url_); }

Response Request::send_by_SSL(int socketFd)
{
    //为ssl加载加密和哈希算法
    SSL_library_init();
    //加载错误码的描述字符串
    SSL_load_error_strings();
    //选择ssl协议版本号，v23同时支持SSLv2,SSLv3或者TLSv1
    auto method = SSLv23_method();
    //创建SSL_CTX结构
    auto ssl_ctx = SSL_CTX_new(method);
    auto ssl = SSL_new(ssl_ctx);
    check<Error>(ssl != nullptr && ssl_ctx != nullptr ,"openssl初始化异常");
    SSL_set_fd(ssl, socketFd);
    SSL_connect(ssl);
    SSL_write(ssl, source_.data(), source_.size());
    auto resp = read_response(socketFd, ssl);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);
    return resp;
}

#ifndef SSL_DISABLE
Response Request::read_response(int socketFd, SSL* ssl = nullptr)
#else
Response Request::read_response(int socketFd)
#endif
{
    const int buf_size = 2048;
    std::array<char, buf_size> buf{0};
    //匿名读取函数
    auto read = [&]()
    {
        buf.fill(0);
        int read_number = 0;
        if(protocol_ == "http")
        {
            read_number = recv(socketFd, buf.data(), buf_size, 0);
        }
#ifndef SSL_DISABLE
        else if (protocol_ == "https")
        {
            read_number = SSL_read(ssl, buf.data(), buf_size);
        }
        #endif
        check<PeerConnectionClose>(read_number != 0);
        check(read_number > 0, "网络异常，socket错误码： " + std::to_string(read_number));
        return read_number;
    };
    Response resp;
    int read_number = 0;
    // 解析头部信息
    while(true)
    {
        read_number = read();
        if( read_number > 0)
        {
            resp.source_.insert(resp.source_.end(), buf.data(), buf.data() + read_number);
        }
        if(resp.can_parse_header())
        {
            break;
        }
    }
    resp.parse_header();

    //继续接收余下的报文信息body
    auto check_recv_end = [&]()
    {
        auto &body = resp.getBody();
        if(resp.is_chunked_)
        {
            if( body.size() < 7)
            {
                return false;
            }
            auto chunked_end_offset = body.size() - 4;
            auto chunked_end_iter = body.begin() + chunked_end_offset;
            auto chunked_end = std::string(chunked_end_iter, chunked_end_iter + 4);
            if( chunked_end != string_handler::crlf_crlf)
            {
                return false;
            }
            auto chunked_start_offset = chunked_end_offset - 1;
            //正常来说，分块结束标志是lf0lflf,必须是单个0，但是不排除有多个0，并不代表真的结束了
            for (auto &i = chunked_start_offset; i >= 2; --i)
            {
                char r = body[i];
                if( r != '0')
                {
                    break;
                }
                if(body[i-1] == '\n' && body[i-2] == '\r')
                {
                    return true;
                }
            }
        }
        else
        {
            return body.size() == resp.content_length_;
        }
        return false;
    };
    while(!check_recv_end())
    {
        read_number = read();
        resp.body_.insert(resp.body_.end(), buf.begin(), buf.begin() + read_number);
    }
    close(socketFd);
    //只有分块编码才需要解析，因为有块数，需要去掉
    resp.parse_body();
    return resp;
}

void Request::build_request()
{
    //构建request请求
    //第一部分：请求行，说明请求类型等数据此前已经添加
    //第二部分：请求头部：附加信息，如host，content-type
    header_.add("Host", host_);
    header_.add("Content-Length", std::to_string(request_body_.size()));
    auto request_target = enable_proxy_ ? url_ : path_;
    std::string source_str = method_ + " " + request_target + " " + protocol_version + string_handler::crlf;
    for(auto& x: header_.getRawData())
    {
        source_str += x.first + ": " + x.second + string_handler::crlf;
    }
    //第三部分必须有空行
    source_str += string_handler::crlf;
    string_handler::str_to_charvector(source_, source_str);
    source_.insert(source_.end(), request_body_.begin(), request_body_.end());
}

Response Request::sendRequest(std::string url){
    check<std::invalid_argument>(method_.length(), "请求方法不存在");
    std::smatch match_result;

#ifndef SSL_DISABLE
    std::regex url_parse(R"(^(http|https)://([\w.]*):?(\d*)(/?.*)$)");
    regex_match(url, match_result, url_parse);
    check<std::invalid_argument>(match_result.size() == 5, "url格式不正确 或 使用了http和https以外的协议");
    protocol_ = match_result[1];
    port_ = match_result[3].length() == 0 ? (protocol_ == "http" ? 80 : 443) : stoi(match_result[3]);
#else
    std::regex url_parse;
    std::regex url_parse(R"(^(http)://([\w.]*):?(\d*)(/?.*)$)");
    regex_match(url, match_result, url_parse);
    check<std::invalid_argument>(match_result.size() == 5, "url格式不正确 或 使用了http和https以外的协议");
    protocol_ = match_result[1];
    port_ = match_result[3].length() == 0 ? 80 : stoi(match_result[3]);
#endif
    check<std::invalid_argument>(!(protocol_ == "https" && enable_proxy_), "https暂时不支持代理");
    host_ = match_result[2];
    path_ = match_result[4].length() == 0 ? "/" : match_result[4].str();
    int socketFd = getSocketFd();
    ip_ = string_handler::hasLetter(host_) ? tansform_hostname_to_IP(host_) : host_;
    try
    {
        socket_connect(socketFd,host_, ip_, enable_proxy_, proxy_, port_);
        build_request();
        if (protocol_ == "http")
        {
            send(socketFd, source_.data(), source_.size(), 0);
            return read_response(socketFd);
        }

#ifndef SSL_DISABLE
        else
        {
            return send_by_SSL(socketFd);
        }
#endif
    }
    catch(Error &e)
    {
        throwException(e.error_info());
    }
    throw Error("unreachable code");
}

Response Request::fetch(std::string url, Method method, Header header, std::string body)
{
    return setUrl(url).setMethod(method).setHeader(header).setBody(body).sendRequest();
}
