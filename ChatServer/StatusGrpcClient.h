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

using message::UserServerReq;
using message::UserServerRsp;
using message::UserUidReq;
using message::UserUidRsp;
using message::StatusService;

class StatusConPool {
public:
	StatusConPool(size_t poolSize, std::string host, std::string port) :
		poolSize_(poolSize), host_(host), port_(port), b_stop(false) {
		for (size_t i = 0; i < poolSize; i++) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());
			connections_.push(StatusService::NewStub(channel));
		}
	}

	~StatusConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) connections_.pop();
	}

	void Close() {
		b_stop = true;
		cond_.notify_all();
	}

	std::unique_ptr<StatusService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
			if (b_stop) {
				return true;//停止则直接返回
			}
			return !connections_.empty();//有连接则返回
		});
		//如果停止则直接返回空指针
		if (b_stop) {
			return  nullptr;
		}
		auto context = std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<StatusService::Stub> conn) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop) return;
		connections_.push(std::move(conn));
		cond_.notify_one();
	}

private:
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::atomic<bool> b_stop;
	std::queue<std::unique_ptr<StatusService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;	
public:
	~StatusGrpcClient() {}
	UserUidRsp GetUserUid(bool is_chat,std::string useruid,std::string name);//根据用户名返回uid
	UserServerRsp GetUserServer(int uid);//根据uid返回服务器信息
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> pool_;

};

