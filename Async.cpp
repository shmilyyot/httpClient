#include "Async.h"


Async& Async::setThreadNum(int num)
{
    thread_num_ = num;
    return *this;
}

Async &Async::setBolck(bool wait)
{
    is_block_ = wait;
    return *this;
}

Async &Async::setThrowIfHasErrMsg(bool op)
{
    throw_if_has_err_msg = op;
    return *this;
}

AsyncResponse Async::get_target_response(int id)
{
    auto &queue = waiting_handle_response_;
    for (size_t i = 0; i < queue.size(); ++i)
    {
        AsyncResponse &target = queue[i];
        if(target.id == id)
        {
            AsyncResponse res = queue[i];
            if(throw_if_has_err_msg && res.err_msg.size())
            {
                throwException(res.err_msg);
            }
            queue.erase(queue.begin() + i);
            return res;
        }
    }
    throw Error("unreachable code");
}

AsyncResponse Async::await(int id, int timeout_ms)
{
    std::unique_lock<std::mutex> lk(waiting_resp_queue_mutex_);
    auto &queue = waiting_handle_response_;
    auto check_response_exist = [&]
    {
        for(AsyncResponse &response : queue)
        {
            if(response.id == id)
            {
                return true;
            }
        }
        return false;
    };
    if(check_response_exist())
    {
        return get_target_response(id);
    }
    if(timeout_ms != 0)
    {
        //timeout_ms之后被唤醒往下执行或者被notify
        waiting_resp_cv_.wait_for(lk, std::chrono::milliseconds(timeout_ms), check_response_exist);
        if(!check_response_exist())
        {
            throw AsyncAwaitTimeout();
        }
    }else
    {
        waiting_resp_cv_.wait(lk, check_response_exist);
    }
    return get_target_response(id);
}

void Async::AsyncLoop(int id)
{
    AsyncPackage pkg;
    while(!stopped_.load())
    {
        std::unique_lock<std::mutex> lk(m_);
        //当没有resquest或客户端停止运行时，唤醒线程
        cv_.wait(lk, [&]
                 { return !queue_.empty() || stopped_.load(); });
        if(stopped_.load())
        {
            break;
        }
        pkg = queue_.front();
        queue_.pop();
        //提前解锁 ，因为接下来马上提醒别的线程可以从队列里面取request了，不解锁别的线程唤醒之后获取不到锁，导致马上又阻塞
        lk.unlock();
        //通知别的线程可以取request了
        cv_.notify_one();
        Response resp;
        std::string err_msg;
        if(stopped_.load())
        {
            break;
        }
        try
        {
            //内部会阻塞直到接收到请求
            resp = pkg.request_.sendRequest();
        }
        catch(const std::exception& e)
        {
            err_msg = e.what();
        }
        if(stopped_.load())
        {
            break;
        }
        AsyncResponse resp_pkg;
        resp_pkg.id = pkg.id_;
        resp_pkg.resp = resp;
        resp_pkg.err_msg = err_msg;
        if (pkg.received_mode_ == AsyncResponseReceiverMode::queue)
        {
            std::lock_guard<std::mutex> m(waiting_resp_queue_mutex_);
            waiting_handle_response_.push_back(resp_pkg);
            waiting_resp_cv_.notify_all();
        }
        else if(pkg.received_mode_ == AsyncResponseReceiverMode::callback)
        {
            try
            {
                pkg.callback_(resp_pkg);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
        }
    }
    {
        //销毁线程
        std::lock_guard<std::mutex> m(m_);
        threads_.erase(id);
        cv_.notify_one();
    }
}

void Async::start()
{
    check<std::logic_error>(!running_, "一个线程池实例只能启动一次");
    for (size_t i = 0; i < thread_num_; ++i)
    {
        threads_[i] = std::thread([&, i]
                                  { AsyncLoop(i); });
    }
    running_ = true;
    if(is_block_)
    {
        for(auto& i : threads_)
        {
            i.second.join();
        }
    }else
    {
        for(auto& i : threads_)
        {
            i.second.detach();
        }
    }
}

int Async::run(std::function<Request()> fn)
{
    auto id = ++incr_id;
    {
        std::lock_guard<std::mutex> lock(m_);
        queue_.emplace(fn(), id, AsyncResponseReceiverMode::queue);
    }
    cv_.notify_one();
    return id;
}

void Async::run(std::function<Request()> fn, std::function<void(AsyncResponse)> cb)
{
    {
        std::lock_guard<std::mutex> lock(m_);
        queue_.emplace(fn(), cb, AsyncResponseReceiverMode::callback);
    }
    cv_.notify_one();
}

std::vector<AsyncResponse> Async::get_available_response()
{
    std::lock_guard<std::mutex> lock(waiting_resp_queue_mutex_);
    auto available_resp_queue = waiting_handle_response_;
    waiting_handle_response_ = {};
    return available_resp_queue;
}
