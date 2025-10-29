#include "AsioServicePool.h"
#include "TcpChatServer.h"
#include "ClientSession.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "ServiceRegistry.h"
#include "global.h"
#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include "ChatServiceImpl.h"

int main() {
	try {
		//��������
		std::string config_path = "E:\\QT_project\\Chat8\\server\\ChatServer\\config.ini";
		if (!ConfigMgr::GetInstance()->Load(config_path)) {
			std::cerr << "Loading file fail,path: "<<config_path << std::endl;
			return 1;
		}
		//��ʼ��redis
		std::string redis_host = ConfigMgr::GetInstance()->GetValue("Redis", "Host");
		int redis_port = std::stoi(ConfigMgr::GetInstance()->GetValue("Redis", "Port"));
		std::string redis_passwd = ConfigMgr::GetInstance()->GetValue("Redis", "Passwd");
		if (!RedisMgr::GetInstance()->Init(redis_host, redis_port, redis_passwd)) {
			std::cerr << "Redis connect failed" << std::endl;
			return 1;
		}
		//��ʼ��io�̳߳�
		AsioServicePool io_pool(4);
		if (!ServiceRegistry::GetInstance()->Init(io_pool.get_io_context())) {
			std::cerr << "����ע�������ʼ��ʧ��" << std::endl;
			return 1;
		}
		ServiceRegistry::GetInstance()->Unregister();//��ֹ�ϴ��쳣�˳�δע��
		ServiceRegistry::GetInstance()->Register();
		ServiceRegistry::GetInstance()->StartHeartbeat();

		//��������������
		unsigned short chat_port = std::stoi(ConfigMgr::GetInstance()->GetValue("ChatServer", "Port"));
		std::string chat_port_str = std::to_string(chat_port);
		current_server = "0.0.0.0:" + chat_port_str;
		TcpChatServer server(io_pool.get_io_context(), chat_port);
		std::cout << "Server Started,port: "<<chat_port<<" waiting for connection" << std::endl;
		

		//����grpc������
		std::string grpc_port = ConfigMgr::GetInstance()->GetValue("ChatServer", "Grpc_Port");
		std::string grpc_host = ConfigMgr::GetInstance()->GetValue("ChatServer", "Host");
		std::string grpc_address = "127.0.0.1:" + grpc_port;
		ChatServiceImpl service(server);
		grpc::ServerBuilder builder;
		builder.AddListeningPort(grpc_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		std::unique_ptr<grpc::Server> server_(builder.BuildAndStart());
		if (!server_) {
			std::cerr << "Failed to start gRPC server at " << grpc_address << std::endl;
			return 1;
		}
		std::cout << "gRPC server started at " << grpc_address << std::endl;


		//�����ж��źš�ctrl+c��,ֹͣ������
		asio::signal_set signals(io_pool.get_io_context(),SIGINT,SIGTERM);
		signals.async_wait([&](error_code, int) {
			io_pool.~AsioServicePool();
			});
		//���߳���������ͣ��ֱ����ʱ
		std::this_thread::sleep_until(std::chrono::system_clock::time_point::max());
	}
	catch(const std::exception& e){
		std::cerr << "Server exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}