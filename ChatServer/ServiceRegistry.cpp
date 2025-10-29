#include "ServiceRegistry.h"
#include <chrono>
#include <sstream>
#include <string>
#include "global.h"

bool ServiceRegistry::Init(asio::io_context& io_ctx)
{
	heartbeat_timer_ = std::make_unique<asio::steady_timer>(io_ctx);
	// 读取配置文件
	host_ = ConfigMgr::GetInstance()->GetValue("ChatServer","Host");
	std::string port_str = ConfigMgr::GetInstance()->GetValue("ChatServer","Port");
	if (port_str.empty()) return false;
	port_ = port_str;
	std::string grpc_port= ConfigMgr::GetInstance()->GetValue("ChatServer", "Grpc_Port");
	//生成服务唯一标识
	service_key_ = "ChatServer:" + host_ + ":" + port_str+":"+grpc_port;
	service_key = service_key_;
	return true;
}

void ServiceRegistry::Register() {
	//构建服务信息
	std::stringstream ss;
	ss << "name=ChatServer;host=" << host_ << ";port=" << port_;
	ss << ";status=running;start_time=" << time(nullptr);
	RedisMgr::GetInstance()->HSet("service_registry",&service_key_, ss.str());
	RedisMgr::GetInstance()->Set(service_key, "0");
	std::cout << service_key << std::endl;
	std::cout << "Service Register Success: " << service_key_ << std::endl;
}

void ServiceRegistry::Unregister() {
	//从redis中删除服务信息
	RedisMgr::GetInstance()->HDel("service_registry", service_key_);
	std::cout << "Service Unregister Success: " << service_key_ << std::endl;
}

void ServiceRegistry::StartHeartbeat() {
	DoHeartbeat();
}	

void ServiceRegistry::DoHeartbeat() {
	//更新心跳时间
	std::stringstream ss;
	ss << "name=ChatServer;host=" << host_ << ";port=" << port_;
	ss << ";status=running;last_heartbeat=" << time(nullptr);
	RedisMgr::GetInstance()->HSet("service_registry",&service_key_,ss.str());
	std::cout << "HeartBeat Update" << service_key_ << std::endl;
	//设置30秒后下一次心跳时间
	heartbeat_timer_->expires_after(std::chrono::seconds(30));
	heartbeat_timer_->async_wait([this](const boost::system::error_code& ec) {
		if (!ec)  DoHeartbeat();
	});
}