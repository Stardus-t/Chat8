#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"
#include <climits>
#include "GateGrpcClient.h"

//std::string generate_unique_string() {
//	// 创建UUID对象
//	boost::uuids::uuid uuid = boost::uuids::random_generator()();
//
//	// 将UUID转换为字符串
//	std::string unique_string = to_string(uuid);
//
//	return unique_string;
//}
//
//Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
//{
//	std::string prefix("llfc status server has received :  ");
//	/*const auto& server = getChatServer();*/
//	reply->set_host(server.host);
//	reply->set_port(server.port);
//	reply->set_error(ErrorCodes::Success);
//	reply->set_token(generate_unique_string());
//	/*insertToken(request->uid(), reply->token());*/
//	return Status::OK;
//}
//
//StatusServiceImpl::StatusServiceImpl()
//{
//	auto& cfg = ConfigMgr::Inst();
//	auto server_list = cfg["chatservers"]["Name"];
//
//	std::vector<std::string> words;
//
//	std::stringstream ss(server_list);
//	std::string word;
//
//	while (std::getline(ss, word, ',')) {
//		words.push_back(word);
//	}
//
//	for (auto& word : words) {
//		if (cfg[word]["Name"].empty()) {
//			continue;
//		}
//
//		ChatServer server;
//		server.port = cfg[word]["Port"];
//		server.host = cfg[word]["Host"];
//		server.name = cfg[word]["Name"];
//		_servers[server.name] = server;
//	}
//
//}
//
//ChatServer StatusServiceImpl::getChatServer() {
//	std::lock_guard<std::mutex> guard(_server_mtx);
//	auto minServer = _servers.begin()->second;
//	
//	//auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
//	//if (count_str.empty()) {
//	//	//不存在则默认设置为最大
//	//	minServer.con_count = INT_MAX;
//	//}
//	//else {
//	//	minServer.con_count = std::stoi(count_str);
//	//}
//
//
//	//// 使用范围基于for循环
//	//for ( auto& server : _servers) {
//	//	
//	//	if (server.second.name == minServer.name) {
//	//		continue;
//	//	}
//
//	//	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
//	//	if (count_str.empty()) {
//	//		server.second.con_count = INT_MAX;
//	//	}
//	//	else {
//	//		server.second.con_count = std::stoi(count_str);
//	//	}
//
//	//	if (server.second.con_count < minServer.con_count) {
//	//		minServer = server.second;
//	//	}
//	//}
//
//	return minServer;
//}
//
//Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
//{
//	auto uid = request->uid();
//	auto token = request->token();
//
//	std::string uid_str = std::to_string(uid);
//	std::string token_key = USERTOKENPREFIX + uid_str;
//	std::string token_value = "";
//	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
//	if (success) {
//		reply->set_error(ErrorCodes::UidInvalid);
//		return Status::OK;
//	}
//	
//	if (token_value != token) {
//		reply->set_error(ErrorCodes::TokenInvalid);
//		return Status::OK;
//	}
//	reply->set_error(ErrorCodes::Success);
//	reply->set_uid(uid);
//	reply->set_token(token);
//	return Status::OK;
//}
//
//void StatusServiceImpl::insertToken(int uid, std::string token)
//{
//	std::string uid_str = std::to_string(uid);
//	std::string token_key = USERTOKENPREFIX + uid_str;
//	RedisMgr::GetInstance()->Set(token_key, token);
//}
std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

void StatusServiceImpl::UpdateServerHealth(const std::string& server_key, bool is_alive) {
	std::lock_guard<std::mutex> lock(_healthy_mtx);
	if (is_alive) {
		//若健康，且不在列表中，初始化连接数为0
		if (_healthy_servers.find(server_key) == _healthy_servers.end())
			_healthy_servers[server_key] = 0;
	}
	//不健康则从列表移除
	else _healthy_servers.erase(server_key);
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
	std::lock_guard<std::mutex> lock(_healthy_mtx);
	if (_healthy_servers.empty()) {
		reply->set_error(ErrorCodes::NoAvaliableServer);
		return Status::OK;
	}
	/*std::string prefix("llfc status server has received :  ");
	std::lock_guard<std::mutex> lock(_mtx);
	_server_index = (_server_index+1) % (_servers.size());
	auto& server = _servers[_server_index];*/
	std::string min_key, user_uid_str;
	int user_uid = request->uid();
	user_uid_str = std::to_string(user_uid);
	//std::cout << "获得请求的Uid: " << user_uid << std::endl;
	int min_count = INT_MAX;
	for (auto& pair : _healthy_servers) {
		const std::string& key = pair.first;
		string count_str = GetServerConnectionCount(key);
		int count = stoi(count_str);
		pair.second = count; // 更新连接数
		if (count < min_count) {
			min_count = count;
			min_key = key;
		}
	}

	// 2. 解析服务器host和port（key格式："ChatServer:host:port"）
	size_t pos1 = min_key.find(":");
	size_t pos2 = min_key.find(":", pos1 + 1);
	size_t pos3 = min_key.find(":", pos2 + 1);
	std::string host = min_key.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string port = min_key.substr(pos2 + 1, pos3 - pos2 - 1);



	// 3. 更新该服务器的连接数（+1）
	std::string count_str = GetServerConnectionCount(min_key);
	//_healthy_servers[min_key]=stoi(count_str);
	//std::cout << "Server: " << min_key << ", Connection Count: " << count_str << std::endl;
	reply->set_host(host);
	reply->set_port(port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	{
		std::lock_guard<std::mutex> lock(user_server_mtx);
		user_server_map[user_uid_str] = min_key; // 记录用户与服务器的映射
		std::cout << "user_uid: " << user_uid_str << " connected to " << min_key << std::endl;
	}
	return Status::OK;
}

Status StatusServiceImpl::GetUserServer(::grpc::ServerContext* context, const ::message::UserServerReq* request, ::message::UserServerRsp* response) {
	std::string user_uid = request->target_uid();
	//std::cout << "获得请求的Uid: " << user_uid << std::endl;
	std::lock_guard<std::mutex> lock(user_server_mtx);
	std::string server_key = user_server_map[user_uid];
	/*for (const auto& pair : user_server_map) {
		std::cout << "键（user_uid）: " << pair.first
			<< ", 值（server_key）: " << pair.second << std::endl;
	}*/
	if (server_key.empty()) {
		response->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}
	// 解析服务器host和port（key格式："ChatServer:host:port"）
	size_t pos1 = server_key.find(":");
	size_t pos2 = server_key.find(":", pos1 + 1);
	size_t pos3 = server_key.find(":", pos2 + 1);
	std::string host = server_key.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string port = server_key.substr(pos2 + 1,pos3-pos2-1);
	std::string grpc_port = server_key.substr(pos3 + 1);
	response->set_host(host);
	response->set_port(port);
	response->set_grpc_port(grpc_port);
	return Status::OK;
}

//根据发送的用户名搜索该用户的uid
grpc::Status StatusServiceImpl::GetUserUid(::grpc::ServerContext* context, const ::message::UserUidReq* request, ::message::UserUidRsp* response) {
	bool is_chat = request->is_chat();
	std::string user_name = request->sender_uid();
	std::string target_name = request->user_name();

	std::cout << "接收到的用户名：" << user_name << " 目标用户名：" << target_name << std::endl;
	//std::cout << "开始发送获得UID请求" << std::endl;
	auto reply = GateGrpcClient::GetInstance()->GetUseruid(is_chat,user_name,target_name);
	//std::cout << "得到回复" << std::endl;
	if (reply.error()) {
		std::cout << "grpc get chat server failed, error is " << reply.error() << std::endl;
		response->set_error(ErrorCodes::RPCGetFailed);
		return Status::OK;
	}
	std::string target_uid = reply.target_uid();
	std::cout << "Target user " << user_name << " has UID: " << target_uid << std::endl;
	response->set_target_uid(target_uid);
	return Status::OK;
}

StatusServiceImpl::StatusServiceImpl() :_server_index(0)
{
	auto& cfg = ConfigMgr::Inst();
	ChatServer server;
	server.port = cfg["ChatServer"]["Port"];
	server.host = cfg["ChatServer"]["Host"];
	server.grpc_port = cfg["ChatServer"]["Grpc_Port"];
	//std::cout << "grpc_port: " << server.grpc_port << std::endl;
	_servers.push_back(server);

	server.port = cfg["ChatServer2"]["Port"];
	server.host = cfg["ChatServer2"]["Host"];
	server.grpc_port = cfg["ChatServer2"]["Grpc_Port"];
	//std::cout << "grpc_port: " << server.grpc_port << std::endl;
	_servers.push_back(server);
}

// 从redis获取指定服务器的连接数
std::string StatusServiceImpl::GetServerConnectionCount(const std::string& server_key) {
	std::string connection_count="0";
	if (!RedisMgr::GetInstance()->Get(server_key, connection_count)) std::cerr << "Failed to get connection count for " << server_key << std::endl;
	return connection_count;
}