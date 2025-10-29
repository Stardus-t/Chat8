#pragma once
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>          
#include <atomic>     
#include <mutex>          
#include <condition_variable>  
#include <memory>   
#include <atomic>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::UserUidReq;
using message::UserUidRsp;
using message::GateService;

class GateConPool {
public:
	GateConPool(size_t poolSize,std::string host,std::string port):
		poolSize_(poolSize),host_(host),port_(port),b_stop(false)
	{
		for (size_t i = 0; i < poolSize; i++) {
			std::shared_ptr<Channel>channel = grpc::CreateChannel(host_ + ":" + port_, 
				grpc::InsecureChannelCredentials());
			connections_.push(GateService::NewStub(channel));
		}
	}

	~GateConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) connections_.pop();
	}

	void Close() {
		b_stop = true;
		cond_.notify_all();
	}

	std::unique_ptr<GateService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock,[this] {
			if (b_stop) return true;
			return !connections_.empty();
			});
		if (b_stop) return nullptr;
		auto context = std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<GateService::Stub> conn) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (b_stop) return;
		connections_.push(std::move(conn));
		cond_.notify_one();
	}

private:
	std::queue<std::unique_ptr<GateService::Stub>> connections_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	atomic<bool> b_stop;
	std::mutex mutex_;
	std::condition_variable cond_;
};

class GateGrpcClient :public Singleton<GateGrpcClient>
{
	friend class Singleton<GateGrpcClient>;
public:
	~GateGrpcClient() {}
	UserUidRsp GetUseruid(bool is_chat,std::string user_name,std::string target_name);
private:
	GateGrpcClient();
	std::unique_ptr<GateConPool> pool_;
};

