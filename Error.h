#include <functional>
#include <string>

class Error :public std::exception
{
    std::string msg;
    public:
        Error(){};
        Error(std::string msg) : msg(msg){};
        ~Error(){};
        const char* what() const noexcept { return msg.c_str(); }
        std::string& error_info(){return msg;}
};

class PeerConnectionClose : public Error
{
  public:
    PeerConnectionClose(std::string msg = "对方关闭了连接") : Error(msg) {}
};

class AsyncAwaitTimeout : public Error
{
  public:
    AsyncAwaitTimeout(std::string msg = "await timeout") : Error(msg) {}
};

template<typename ExceptionType = Error>
void throwException(std::string msg = "发生了异常"){ throw ExceptionType(msg); }

template<typename ExceptionType = Error>
void check(bool condition, std::string msg = "", std::function<void()> recycle = []{})
{
    if(!condition)
    {
        recycle();
        throwException<ExceptionType>(msg);
    }
}