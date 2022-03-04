#include "EHttpClient.h"
#include "Async.h"
#include <fstream>
#include <string>
#include <vector>

using namespace EHttpClient;
using namespace std;

Async async_thread_pool;

void AsyncAwait()
{
	//队列模式发送request
	{
		//往队列里塞一个请求，并唤醒一个线程来处理请求，获得相应返回到响应队列
		auto id = async_thread_pool.run([=]
										{ return Request()
												.setUrl("https://blog.csdn.net/rzytc/article/details/50647095")
												.setMethod(Method::GET)
												.setHeader("Content-Type", "application/json; charset=utf-8"); });
		//主线程获得请求锁，并且取出对应id的一个响应
		auto pkg = async_thread_pool.await(id);
		std::cout << "AsyncAwait " << pkg.resp.getHeader().getRawData().size() << pkg.err_msg << std::endl;
	}
	{
		try{
			auto id = async_thread_pool.run([=]
										{ return Request()
												.setUrl("https://blog.csdn.net/rzytc/article/details/50647095")
												.setMethod(Method::GET)
												.setHeader("Content-Type", "application/json; charset=utf-8"); });
			auto pkg = async_thread_pool.await(id,10);
		}catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
}

void AsyncCallback()
{
    async_thread_pool.run([=] { return Request()
												.setUrl("https://blog.csdn.net/rzytc/article/details/50647095")
												.setMethod(Method::GET)
												.setHeader("Content-Type", "application/json; charset=utf-8"); },
                          [](AsyncResponse async_resp) { std::cout << "AsyncCallback " << async_resp.resp.getStatus() << std::endl; });
}

int main(){
	async_thread_pool.start();
	AsyncAwait();
	cout<< "----------------------------------------"<<endl;
	AsyncCallback();
	sleep(2000);
	return 1;
}