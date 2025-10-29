#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>  
#include <deque>
class ClientSession;
#include "ClientSession.h"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using error_code = boost::system::error_code;

class TcpChatServer
{
public:
	TcpChatServer(asio::io_context& io_ctx, unsigned short port) :
		io_ctx_(io_ctx), acceptor_(io_ctx, tcp::endpoint(tcp::v4(), port)) {
		if (!acceptor_.is_open()) {
			throw std::runtime_error("Failed to open acceptor on port " + std::to_string(port));
		}
		do_acceptor();
	}

	//���Session�������б�
	void add_session(std::shared_ptr<ClientSession> session);
	
	//�������б��Ƴ�Session
	void remove_client(std::shared_ptr<ClientSession> session);

	//�㲥�û��б�
	void broadcast_user_list();
	void broadcast_user_list_no_lock();

	//����û���redis
	void add_user_to_redis(int num_users);
	//�㲥������Ϣ
	void broadcast_public_msg(const std::string &msg,std::shared_ptr<ClientSession> sender);

	//����˽����Ϣ
	bool send_private_msg(bool &is_chat,const std::string user_uid,const std::string &target_name,const std::string &msg, std::shared_ptr<ClientSession> sender);

	//����û��Ƿ����
	bool check_user(bool &is_chat,const std::string user_uid,const std::string& target_name, std::shared_ptr<ClientSession> sender);

	//������ʷ��Ϣ
	void save_history_msg(const std::string& sender, const std::string& recver, const std::string msg);

	//��ȡ��ʷ��Ϣ
	std::vector<std::string> get_history_msgs(const std::string& sender, const std::string& recver, int count = 100);

	int num_clients() {
		std::lock_guard<std::mutex> lock(mtx_);
		return sessions.size();
	}

	const std::vector<std::shared_ptr<ClientSession>>& get_sessions() const {
		return sessions;
	}

	std::mutex& get_mutex() {
		return mtx_;
	}

private:
	//����������
	void do_acceptor();

	asio::io_context& io_ctx_;
	tcp::acceptor acceptor_;
	std::vector<std::shared_ptr<ClientSession>> sessions;
	std::mutex mtx_;
};

