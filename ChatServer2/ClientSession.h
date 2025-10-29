#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>  
#include <deque>
class TcpChatServer;

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using error_code = boost::system::error_code;

//�ͻ��˻Ự�ࣺ�������ͻ��˵��첽��д������״̬���û������첽��ȡ�ͻ������ݣ��첽������Ϣ������������������
class ClientSession : public std::enable_shared_from_this<ClientSession >{
public:
	//��I/O�����ģ�����������
	ClientSession(asio::io_context &io_ctx,TcpChatServer& server):
		sock_(io_ctx),server_(server),name_("unname"){ }
	
	//��ȡsocket
	tcp::socket& socket() { return sock_; }
	
	// �����Ự����ʼ������
	void start() {
		do_read();
	}

	//�첽д�벢������Ϣ
	void async_send(const std::string& msg) {
		uint32_t content_len = static_cast<uint32_t>(msg.size());
		uint32_t len = htonl(content_len);
		std::string send_msg;
		send_msg.append(reinterpret_cast<const char*>(&len), 4);
		send_msg.append(msg);
		asio::post(sock_.get_executor(), [self = shared_from_this(), send_msg]() {
			bool is_write_in_progress = !self->write_bufs_.empty();
			self->write_bufs_.push_back(send_msg);
			if (!is_write_in_progress)  self->do_write();
			});
		std::cout << "���������Ͷ����ƣ�ʮ�����ƣ���";
		for (unsigned char c : send_msg) {
			printf("%02X", c); // ��ӡΪ 00000019555345524c4953547c3836393030333135384071712e636f6d
		}
		std::cout << std::endl;

	}

	//��ȡ�û���
	std::string get_name() const { return name_; }
	
	//�����û���
	void set_name(const std::string& name) { name_ = name; }

	//�ر�����
	void close() {
		if (sock_.is_open()) {
			error_code ec;
			sock_.shutdown(tcp::socket::shutdown_both, ec);
			asio::post(sock_.get_executor(), [self = shared_from_this()]() {
				self->sock_.close();
			});
		}			
	}

private:
	void do_read();
	void do_write();
	void handle_read_msg(const std::string& msg);

	tcp::socket sock_;
	TcpChatServer& server_;
	std::string name_;
	char read_buf_[1024] = { 0 };
	std::deque<std::string> write_bufs_;
};

