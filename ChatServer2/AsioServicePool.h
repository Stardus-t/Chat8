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
		//��ʼ��ÿ��io_context
		for (std::size_t i = 0; i < pool_size; i++) {
			//����io_context���󣬲���ֹ���ͻ��˶��˳���io_context�������ص��·���������
			auto io_ctx = std::make_shared<asio::io_context>();
			auto work_guard = std::make_shared<asio::executor_work_guard<asio::io_context::executor_type>>(
				io_ctx->get_executor()
			);
			io_ctxs_.push_back(io_ctx);
			work_guards_.push_back(work_guard);
		}
		//����ÿ���߳�,����ÿ���̷߳Ž��̳߳�
		for (std::size_t i = 0; i < pool_size; i++) {
			threads_.emplace_back([this, i] {
				io_ctxs_[i]->run();
			});
		}
	}

	~AsioServicePool() {
		//��������WorkGuard,��io_context.run()��������
		work_guards_.clear();
		//�ȴ������߳��˳�
		for(auto &thread:threads_)
			if (thread.joinable()) thread.join();
	}

	//��ȡһ��io_context
	asio::io_context& get_io_context() {
		auto& io_ctx = *io_ctxs_[next_io_ctx_idx_];
		next_io_ctx_idx_ = (next_io_ctx_idx_ + 1) % io_ctxs_.size();
		return io_ctx;
	}

private:
	std::atomic<std::size_t> next_io_ctx_idx_;//�������Ϊio_context���������Ͳ��������Դ����
	std::vector<std::shared_ptr<asio::io_context>> io_ctxs_;
	std::vector<std::shared_ptr<asio::executor_work_guard<asio::io_context::executor_type>>> work_guards_;
	std::vector<std::thread>threads_;
};

