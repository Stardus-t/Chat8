#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>  
#include <deque>
#include <thread>
#include <atomic>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using error_code = boost::system::error_code;

class AsioServicePool
{
public:
	explicit AsioServicePool(std::size_t pool_size = std::thread::hardware_concurrency()) :next_io_ctx_idx_(0) {
		if (pool_size == 0) pool_size = 1;
		//初始化每个io_context
		for (std::size_t i = 0; i < pool_size; i++) {
			//创建io_context对象，并防止当客户端都退出后，io_context立即返回导致服务器断线
			auto io_ctx = std::make_shared<asio::io_context>();
			auto work_guard = std::make_shared<asio::executor_work_guard<asio::io_context::executor_type>>(
				io_ctx->get_executor()
			);
			io_ctxs_.push_back(io_ctx);
			work_guards_.push_back(work_guard);
		}
		//启动每个线程,并将每个线程放进线程池
		for (std::size_t i = 0; i < pool_size; i++) {
			threads_.emplace_back([this, i] {
				io_ctxs_[i]->run();
			});
		}
	}

	~AsioServicePool() {
		//销毁所有WorkGuard,让io_context.run()函数返回
		work_guards_.clear();
		//等待所有线程退出
		for(auto &thread:threads_)
			if (thread.joinable()) thread.join();
	}

	//获取一个io_context
	asio::io_context& get_io_context() {
		auto& io_ctx = *io_ctxs_[next_io_ctx_idx_];
		next_io_ctx_idx_ = (next_io_ctx_idx_ + 1) % io_ctxs_.size();
		return io_ctx;
	}

private:
	std::atomic<std::size_t> next_io_ctx_idx_;//用这个作为io_context的索引，就不会产生资源竞争
	std::vector<std::shared_ptr<asio::io_context>> io_ctxs_;
	std::vector<std::shared_ptr<asio::executor_work_guard<asio::io_context::executor_type>>> work_guards_;
	std::vector<std::thread>threads_;
};

