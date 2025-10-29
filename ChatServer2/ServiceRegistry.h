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
	bool Init(asio::io_context& io_ctx);//��ȡ�����ļ�������redis����io������
	void Register();//ע�����
	void Unregister();//ע������
	void StartHeartbeat();//��������
protected:
	friend class Singleton<ServiceRegistry>;
private:	
	void DoHeartbeat();
	std::unique_ptr<asio::steady_timer> heartbeat_timer_;
	std::string service_key_;//����Ψһ��ʶ
	std::string host_;// ��������
	std::string port_;// ����˿�
};

