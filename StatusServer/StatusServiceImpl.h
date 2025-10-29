#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>
#include "message.pb.h"   
#include <string>
#include <unordered_map>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
//using message::LoginReq;
//using message::LoginRsp;
using message::StatusService;

class  ChatServer {
public:
	/*ChatServer():host(""),port(""),name(""),con_count(0){}
	ChatServer(const ChatServer& cs):host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count){}
	ChatServer& operator=(const ChatServer& cs) {
		if (&cs == this) {
			return *this;
		}

		host = cs.host;
		name = cs.name;
		port = cs.port;
		con_count = cs.con_count;
		return *this;
	}*/
	std::string host;
	std::string port;
	std::string grpc_port;
	/*std::string name;
	int con_count;*/
};
class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();
	Status GetUserUid(::grpc::ServerContext* context, const ::message::UserUidReq* request, ::message::UserUidRsp* response) override;
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply) override;
	Status GetUserServer(::grpc::ServerContext* context,const ::message::UserServerReq* request,::message::UserServerRsp* response) override;
	std::vector<ChatServer> _servers;
	int _server_index;
	std::mutex _mtx;
	std::unordered_map<std::string, int> _healthy_servers;
	std::mutex _healthy_mtx;//保护健康列表的线程安全

	std::unordered_map<std::string, std::string> user_server_map ;	
	std::mutex user_server_mtx;

	void UpdateServerHealth(const std::string& server_key,bool is_alive);

	std::string GetServerConnectionCount(const std::string& server_key);

	//Status Login(ServerContext* context, const LoginReq* request,
	//	LoginRsp* reply) override;
//private:
//	void insertToken(int uid, std::string token);
//	ChatServer getChatServer();
//	std::unordered_map<std::string, ChatServer> _servers;
//	std::mutex _server_mtx;

};

