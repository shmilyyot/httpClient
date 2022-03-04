#pragma once
#include <queue>
#include <map>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <functional>
#include <iostream>

#include "Response.h"
#include "Request.h"

enum AsyncResponseReceiverMode
{
    callback,
    queue,
    future
};

struct AsyncResponse
{
    Response resp;
    int id;
    std::string err_msg;
};

struct AsyncPackage
{
    Request request_;
    std::function<void(AsyncResponse)> callback_;
    int id_;
    AsyncResponseReceiverMode received_mode_;
    AsyncPackage(){};
    AsyncPackage(Request request, int id, AsyncResponseReceiverMode received_mode) : request_(request), id_(id), received_mode_(received_mode){};
    AsyncPackage(Request request, std::function<void(AsyncResponse)> callback, AsyncResponseReceiverMode received_mode) : request_(request), callback_(callback), received_mode_(received_mode){};
};

class Async{
    int thread_num_ = 6;
    std::queue<AsyncPackage> queue_;
    std::mutex m_;
    std::condition_variable cv_;
    std::condition_variable waiting_resp_cv_;
    std::unordered_map<int, std::thread> threads_;
    bool running_ = false;
    bool throw_if_has_err_msg = false;
    std::atomic_bool stopped_{false};
    std::atomic_int incr_id{0};
    bool is_block_ = false;
    std::mutex waiting_resp_queue_mutex_;
    std::vector<AsyncResponse> waiting_handle_response_;

    public:
        ~Async()
        {
            stopped_ = true;
            cv_.notify_all();
            std::unique_lock<std::mutex> lk(m_);
            auto &ts = threads_;
            cv_.wait(lk, [&]
                     { return ts.empty(); });
        }
        Async &setThreadNum(int num);
        Async &setBolck(bool wait);
        Async &setThrowIfHasErrMsg(bool op);
        AsyncResponse get_target_response(int id);
        AsyncResponse await(int id, int timeout_ms = 0);
        void start();
        int run(std::function<Request()> fn);
        void run(std::function<Request()> fn, std::function<void(AsyncResponse)> cb);
        std::vector<AsyncResponse> get_available_response();

    private:
        void AsyncLoop(int id);
};