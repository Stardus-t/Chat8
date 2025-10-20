#pragma once
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "RedisMgr.h"
#include "ConfigMgr.h"

enum class ServiceType {
	CHAT_SERVER,
	GATE_SERVER,
	CHAT_SERVER2
};

class ServiceHealthChecker
{
public:
	using CheckResultCallback = std::function<void(bool)>;
	ServiceHealthChecker(ServiceType type, const std::string& host,const std::string& port,CheckResultCallback callback,boost::asio::io_context& io_ctx);
	//开始/停止检查
	void StartChecking(int interval=30);
	void StopChecking();
	//从配置文件获取服务的主机和端口
	bool GetServiceHostAndPort(std::string& host, std::string& port, std::string &grpc_port);
private:
	void DoCheck();
	void OnTimer(const boost::system::error_code &ec);
	//构建目标服务的唯一标识
	std::string GetServiceKey();
	
	ServiceType service_type;//检查的服务类型
	CheckResultCallback result_callback_;//检查结果回调
	boost::asio::io_context io_ctx_;
	boost::asio::steady_timer timer_;
	int check_interval_;//检查间隔
	bool is_running_;
	std::string host_;
	std::string port_;
};

