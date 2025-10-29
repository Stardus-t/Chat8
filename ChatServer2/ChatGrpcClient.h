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
using message::ChatingReq;
using message::ChatingRsp;
using message::ChatService;

class ChatConPool {
public:
	ChatConPool(size_t poolSize) :
		poolSize_(poolSize), b_stop(false) {}
	
	~ChatConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connectionPools_.empty()) {
			connectionPools_.clear();
		}
	}

	void Close() {
		if (b_stop) return;
		cond_.notify_all();
	}

	std::unique_ptr<ChatService::Stub> getConnection(const std::string &host,const std::string port) {
		std::unique_lock<std::mutex> lock(mutex_);
		std::string key = host + ":" + port;

		if (connectionPools_.find(key) == connectionPools_.end()) {
			connectionPools_[key] == std::queue < std::unique_ptr<ChatService::Stub> > ();
			for (size_t i = 0; i < poolSize_; i++) {
				auto channel = grpc::CreateChannel(key, grpc::InsecureChannelCredentials());
				connectionPools_[key].push(ChatService::NewStub(channel));
			}
		}

		cond_.wait(lock, [this,&key] {
			if (b_stop) return true;
			return !connectionPools_[key].empty();
			});

		if (b_stop) return nullptr;
		auto stub = std::move(connectionPools_[key].front());
		connectionPools_[key].pop();
		return stub;
	}

	void returnConnection(std::unique_ptr<ChatService::Stub>context,const std::string host,const std::string port) {
		std::lock_guard<std::mutex> lock(mutex_);
		
		if (b_stop) return;
		std::string key = host + ":" + port;

		if (connectionPools_.find(key) != connectionPools_.end()) {
			connectionPools_[key].push(std::move(context));
			cond_.notify_one();
		}
	}
	


private:
	std::mutex mutex_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::atomic<bool> b_stop;
	// 用map存储不同地址的连接池（key: host:port）
	std::unordered_map<std::string, std::queue<std::unique_ptr<ChatService::Stub>>> connectionPools_;
	std::condition_variable cond_;
};

class ChatGrpcClient:public Singleton<ChatGrpcClient>
{
	friend class Singleton<ChatGrpcClient>;
public:
	~ChatGrpcClient() {}
	ChatingRsp GetReplyMessage(const std::string& target_host, const std::string& target_port, std::string sender_name, int sender_uid,
		std::string recv_name, int recv_uid, std::string context);
private:
	ChatGrpcClient();
	int poolSize = std::stoi(ConfigMgr::GetInstance()->GetValue("ChatServer", "Grpc_PoolSize"));
	
	std::unique_ptr<ChatConPool> pool_;
	
};

