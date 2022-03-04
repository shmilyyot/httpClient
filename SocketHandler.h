#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>

#include "Error.h"
#include "HttpProxy.h"
#include "String_handler.h"

#ifndef SSL_DISABLE
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif


//获取一个新的文件描述符
int getSocketFd();

//socket连接host
void socket_connect(int socketFd, std::string &host, std::string &ip, bool enable_proxy, HttpProxy& proxy, int port);

//根据主机地址获取ip
std::string tansform_hostname_to_IP(std::string hostname);

