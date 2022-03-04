#include "SocketHandler.h"
#include <cstring>
#include <iostream>

std::string tansform_hostname_to_IP(std::string hostname)
{
    addrinfo hints, *res;
    in_addr addr;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    //处理名字到地址以及服务到端口这两种转换，返回的是一个addrinfo的结构
    int err = 0;
    if((err = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0)
    {
        throwException<Error>("错误" + std::to_string(err) + std::string(gai_strerror(err)));
    }
    addr.s_addr = ((sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    char ip_str[INET_ADDRSTRLEN];
    //ip地址二进制结果转换为字符串
    inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str));
    freeaddrinfo(res);
    return ip_str;
}

//获取一个新的文件描述符
int getSocketFd(){
    //面向网络ipv4,TCP套接字的名字SOCK_STREAM
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    return tcp_socket;
}

//socket连接host
void socket_connect(int socketFd, std::string &host, std::string &ip, bool enable_proxy, HttpProxy& proxy, int port){
    in_addr ia;
    std::string target_ip = enable_proxy ? (string_handler::hasLetter(proxy.host_) ? tansform_hostname_to_IP(proxy.host_) : proxy.host_): ip;
    check<std::invalid_argument>((inet_pton(AF_INET, target_ip.c_str(), &ia) != -1), "地址转换错误");
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(enable_proxy ? proxy.port_ : port);
    saddr.sin_addr = ia;
    if (connect(socketFd, (sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        std::string err = "连接失败:\n";
        err += "Host:" + host + "\n" + "Ip:" + ip + "\n";
        if (enable_proxy)
        {
            err += "Proxy IP:" + target_ip + "\n" + "Proxy Host:" + proxy.host_ + "\n";
        }
        err += std::string("error str:") + strerror(errno);
        throwException(err);
    }
}