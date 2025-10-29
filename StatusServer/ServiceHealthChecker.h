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
	//��ʼ/ֹͣ���
	void StartChecking(int interval=30);
	void StopChecking();
	//�������ļ���ȡ����������Ͷ˿�
	bool GetServiceHostAndPort(std::string& host, std::string& port, std::string &grpc_port);
private:
	void DoCheck();
	void OnTimer(const boost::system::error_code &ec);
	//����Ŀ������Ψһ��ʶ
	std::string GetServiceKey();
	
	ServiceType service_type;//���ķ�������
	CheckResultCallback result_callback_;//������ص�
	boost::asio::io_context io_ctx_;
	boost::asio::steady_timer timer_;
	int check_interval_;//�����
	bool is_running_;
	std::string host_;
	std::string port_;
};

