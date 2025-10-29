#include "TcpChatServer.h"
#include "global.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "ChatGrpcClient.h"

void TcpChatServer::do_acceptor() {
	//创建新的Session
	auto new_session = std::make_shared<ClientSession>(io_ctx_, *this);
	acceptor_.async_accept(new_session->socket(),
		[this, new_session](error_code ec) {
			if (!ec) {
				add_session(new_session);
				new_session->start();
			}
			else 
				std::cerr << "Accept error: " << ec.message() << std::endl;
			do_acceptor();
		});
}
void TcpChatServer::add_session(std::shared_ptr<ClientSession> session) {
	std::lock_guard<std::mutex> lock(mtx_);
	sessions.push_back(session);
	std::cout << "New client connected, current connection: " << sessions.size() << std::endl;
	add_user_to_redis(sessions.size());
	std::cout << "Current connection updated in Redis." << std::endl;
}

void TcpChatServer::remove_client(std::shared_ptr<ClientSession> session) {
	std::lock_guard<std::mutex> lock(mtx_);
	sessions.erase(std::remove(sessions.begin(), sessions.end(), session), sessions.end());
	broadcast_user_list_no_lock();
	std::cout << "Client disconnected, current connection: " << sessions.size() << std::endl;
	current_connection = sessions.size();
	add_user_to_redis(sessions.size());
	std::cout << "Current connection updated in Redis." << std::endl;
}

void TcpChatServer::add_user_to_redis(int num_users) {;
	std::string num = std::to_string(num_users);
	RedisMgr::GetInstance()->Set(service_key,num);
}

void TcpChatServer::broadcast_user_list() {
	std::lock_guard<std::mutex> lock(mtx_);
	broadcast_user_list_no_lock();
}	

void TcpChatServer::broadcast_user_list_no_lock() {
	/*std::lock_guard<std::mutex> lock(mtx_);*/
	std::stringstream ss;
	ss << "USERLIST";
	for (const auto& session : sessions) ss << "|" << session->get_name();
	std::string user_list = ss.str();
	//std::cout << "Broadcast user list: " << user_list << std::endl;
	for (const auto& session : sessions) session->async_send(user_list);
}

void TcpChatServer::broadcast_public_msg(const std::string &msg, std::shared_ptr<ClientSession> sender) {
	std::lock_guard<std::mutex> lock(mtx_);
	for (const auto& session : sessions) 
		if(session!=sender) session->async_send(msg + "\n");
}

bool TcpChatServer::send_private_msg(bool &is_chat,const std::string user_uid,const std::string& target_name, const std::string& msg, std::shared_ptr<ClientSession> sender){
	//通过RPC获取目标用户的UID
	std::cout << "发送获得UID请求" << std::endl;
	auto reply = StatusGrpcClient::GetInstance()->GetUserUid(is_chat,user_uid,target_name);
	if (reply.error()) {
		std::cout << "get uid failed, error is " << reply.error() << std::endl;
		return false;
	}
	std::string target_uid = reply.target_uid();
	std::cout << "Target user " << target_name << " has UID: " << target_uid << std::endl;

	//通过 uid 获取目标用户所在的服务器信息
	int uid = std::stoi(target_uid);
	auto reply2 = StatusGrpcClient::GetInstance()->GetUserServer(uid);
	if (reply2.error()) {
		std::cout << " grpc  server failed, error is " << reply2.error() << std::endl;
		return false;
	}
	std::string target_server = reply2.host() + ":" + reply2.port();
	/*std::cout << "Target user " << target_name << " is on server: " << target_server << std::endl;
	std::cout << "当前server" << current_server << std::endl;*/
	if (target_server == current_server) {
		
		std::lock_guard<std::mutex> lock(mtx_);
		for (const auto& session : sessions) {
			if (session->get_name() == target_name) {
				session->async_send(msg);
				return true;
			}
		}
	}
	else {
		std::string sender_name=sender->get_name();
		int sender_uid=-1;
		auto reply3 = ChatGrpcClient::GetInstance()->GetReplyMessage("127.0.0.1", reply2.grpc_port(), sender_name, sender_uid, target_name, uid, msg);
		if (reply3.error() == 0) {
			return true; // 跨服发送成功
		}
		else {
			std::cout << "Cross-server message failed, error: " << reply3.error() << std::endl;
			return false;
		}
	}
	
	return false;
}

bool TcpChatServer::check_user(bool &is_chat,const std::string user_uid,const std::string& target_name, std::shared_ptr<ClientSession> sender) {
	//通过RPC获取目标用户的UID
	//std::cout << "发送获得UID请求" << std::endl;
	auto reply = StatusGrpcClient::GetInstance()->GetUserUid(is_chat,user_uid,target_name);
	if (reply.error()) {
		std::cout << "get uid failed, error is " << reply.error() << std::endl;
		return false;
	}
	std::string target_uid = reply.target_uid();
	std::cout << "Target user " << target_name << " has UID: " << target_uid << std::endl;

	//通过 uid 获取目标用户所在的服务器信息
	int uid = std::stoi(target_uid);
	auto reply2 = StatusGrpcClient::GetInstance()->GetUserServer(uid);
	if (reply2.error()) {
		std::cout << " grpc  server failed, error is " << reply2.error() << std::endl;
		return false;
	}
	std::string target_server = reply2.host() + ":" + reply2.port();
	std::cout << "Target user " << target_name << " is on server: " << target_server << std::endl;
	std::cout << "当前server" << current_server << std::endl;

}

void TcpChatServer::save_history_msg(const std::string& sender, const std::string& recver, const std::string content) {
	time_t timestamp = time(nullptr);
	std::string msg = "[" + std::to_string(timestamp) + "]" + sender + ":" + content;
	std::string key1 = "history:" + sender + ":" + recver;
	std::string key2 = "history:" + recver + ":" + sender;
	bool ret1 = RedisMgr::GetInstance()->ZAdd(key1,(long long)timestamp,msg);
	bool ret2 = RedisMgr::GetInstance()->ZAdd(key2, (long long)timestamp, msg);
	std::cout << "保存key:" << key1 << std::endl;
	std::cout << "保存key:" << key2 << std::endl;
	if(!ret1||!ret2) std::cerr << "Failed to save history message to Redis" << std::endl;

}

std::vector<std::string> TcpChatServer::get_history_msgs(const std::string& sender, const std::string& recver, int count) {
	std::vector<std::string> msgs;
	std::string key = "history:" + sender + ":" + recver;
	std::cout << "查询key:" << key << std::endl;
	msgs = RedisMgr::GetInstance()->ZRange(key,std::max(0,-count), -1);
	return msgs;
}
