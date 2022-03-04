#pragma once

#include <string>
#include <vector>
#include <regex>

#include "Header.h"
#include "String_handler.h"
#include "EHttpClient.h"
#include "HttpProxy.h"
#include "Response.h"
#include "SocketHandler.h"
#include "PayLoad.h"


using namespace EHttpClient;

class Request{
        std::string method_;
        std::vector<char> source_;
        std::string path_;
        std::string protocol_;
        std::string url_;
        std::string ip_;
        std::string host_;
        std::vector<char> request_body_;
        std::string protocol_version = "HTTP/1.1";
        Header header_;
        HttpProxy proxy_;
        bool enable_proxy_ = false;
        int port_ = 80;            

        public:
            Request(){};
            ~Request(){};
            Request& setMethod(Method method);
            Request& setUrl(std::string url);
            Request& setBody(std::string body);
            Request& setBody(payload::Binary body);
            Request& setBody(payload::FormData body);
            std::vector<char>& getBody();
            Request& setHeader(Header header);
            Request& setHeader(std::string key, std::string value);
            Header& getHeader();
            Request& setProxy(HttpProxy proxy);
            Response sendRequest(Method method, std::string url);
            Response sendRequest(std::string url);
            Response sendRequest();
            Response fetch(std::string url, Method method = Method::GET, Header header = Header(), std::string body = "");

        private:
            Response send_by_SSL(int socketFd);
            #ifndef SSL_DISABLE
            Response read_response(int socketFd, SSL* ssl);
            #else
            Response read_response(int socketFd);
            #endif
            void build_request();
};