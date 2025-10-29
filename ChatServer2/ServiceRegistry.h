#pragma once
#include "Singleton.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include <boost/asio.hpp>
#include <string>
#include <ctime>

namespace asio = boost::asio;

class ServiceRegistry:public Singleton<ServiceRegistry>{
public:
	bool Init(asio::io_context& io_ctx);//读取配置文件，连接redis，绑定io上下文
	void Register();//注册服务
	void Unregister();//注销服务
	void StartHeartbeat();//启动心跳
protected:
	friend class Singleton<ServiceRegistry>;
private:	
	void DoHeartbeat();
	std::unique_ptr<asio::steady_timer> heartbeat_timer_;
	std::string service_key_;//服务唯一标识
	std::string host_;// 服务主机
	std::string port_;// 服务端口
};

