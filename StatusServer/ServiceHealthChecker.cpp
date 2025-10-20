#include "ServiceHealthChecker.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include <ctime>
#include <iostream>

ServiceHealthChecker::ServiceHealthChecker(ServiceType type, const std::string& host,const std::string& port,CheckResultCallback callback,boost::asio::io_context& io_ctx) :
	service_type(type), host_(host),port_(port),result_callback_(callback), timer_(io_ctx_), check_interval_(30), is_running_(false){}

void ServiceHealthChecker::StartChecking(int interval) {
	if (is_running_) {
		return;
	}
	check_interval_ = interval;
	is_running_ = true;
	DoCheck();
	std::thread([this]() { io_ctx_.run(); }).detach();
}

void ServiceHealthChecker::StopChecking() {
	is_running_ = false;
	timer_.cancel();
	io_ctx_.stop();
}

void ServiceHealthChecker::DoCheck() {
	bool is_alive = false;
	std::string service_key = GetServiceKey();
	std::cout << "service_key: " << service_key << std::endl;
	std::string status;
	if (!service_key.empty()) {
		status = RedisMgr::GetInstance()->HGet("service_registry", service_key);
		if (status.empty()) {
			std::cerr << "No such service registered: " << service_key << std::endl;
			return;
		}
		size_t pos = status.find("last_heartbeat=");
		if (pos != std::string::npos) {
			try {
				time_t last_heartbeat = std::stol(status.substr(pos + 15));
				time_t now = std::time(nullptr);
				is_alive = (now - last_heartbeat) <= (check_interval_ * 2);
			}
			catch (...) {
				std::cerr << "Invalid heartbeat timestamp for service: " << service_key << std::endl;
			}
		}
	}
	if (result_callback_) result_callback_(is_alive);
	if (is_running_) {
		timer_.expires_after(std::chrono::seconds(check_interval_));
		timer_.async_wait(std::bind(&ServiceHealthChecker::OnTimer, this, std::placeholders::_1));
	}
}

void ServiceHealthChecker::OnTimer(const boost::system::error_code& ec) {
	if (ec) {
		std::cerr<<"Timer error: " << ec.message() << std::endl;
		return;
	}
	if (is_running_) DoCheck();
}

std::string ServiceHealthChecker::GetServiceKey() {
	std::string host, port,grpc_port;
	if (!GetServiceHostAndPort(host, port,grpc_port)) {
		return "";
	}
	switch (service_type) {
		case ServiceType::GATE_SERVER:
			return "GateServer:" + host + ":" + port+":"+grpc_port;
		case ServiceType::CHAT_SERVER:
			return "ChatServer:" + host + ":" + port + ":" + grpc_port;
		default:
			return "";
	}
}

bool ServiceHealthChecker::GetServiceHostAndPort(std::string &host,std::string &port,std::string &grpc_port) {
	switch (service_type) {
		case ServiceType::GATE_SERVER:
			host = ConfigMgr::Inst().GetValue("GateServer", "Host");
			port = ConfigMgr::Inst().GetValue("GateServer", "Port");
			grpc_port= ConfigMgr::Inst().GetValue("GateServer", "Grpc_Port");
			//std::cout << "grpc_port: " << grpc_port << std::endl;
			break;
		case ServiceType::CHAT_SERVER:
			host = ConfigMgr::Inst().GetValue("ChatServer", "Host");
			port = ConfigMgr::Inst().GetValue("ChatServer", "Port");
			grpc_port = ConfigMgr::Inst().GetValue("ChatServer", "Grpc_Port");
			//std::cout << "grpc_port: " << grpc_port << std::endl;
			break;
		default:
			return false;
	}
	return !(host.empty() || port.empty());
}

