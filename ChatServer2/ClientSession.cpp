#include "ClientSession.h"
#include "TcpChatServer.h"

//异步读取数据
void ClientSession::do_read() {
	auto self = shared_from_this();//防止读操作还没完成，ClientSession对象被销毁
	//异步读取数据
	sock_.async_read_some(asio::buffer(read_buf_, sizeof(read_buf_) - 1),
		[self](error_code ec, std::size_t bytes_read) {
			if (!ec) {
				self->read_buf_[bytes_read] = '\0';
				std::string msg(self->read_buf_);
				self->handle_read_msg(msg);
				self->do_read();
			}
			else {
				std::cout << "Client disconnected: " << self->name_ << " (" << ec.message() << ")\n";
				self->close();
				self->server_.remove_client(self);
			}
		});
}

//异步发送数据
void ClientSession::do_write() {
	auto self = shared_from_this();
	asio::async_write(sock_, asio::buffer(self->write_bufs_.front()),
		[self](error_code ec, std::size_t) {
			if (!ec) {
				self->write_bufs_.pop_front();//去掉已经发送的消息
				if (!self->write_bufs_.empty()) self->do_write();//继续发送下一条消息
			}
			else self->server_.remove_client(self);
		});
}

void ClientSession::handle_read_msg(const std::string& msg) {
	std::cout << "收到消息: "<<msg << std::endl;
	//设置用户名
	if (name_ == "unname") {
		set_name(msg);
		server_.broadcast_user_list();
	}
	else {
		//已经设置用户名，处理公聊/私聊消息
		if (msg.substr(0, 1) == "@") {
			size_t space_pos = msg.find(' ');
			if (space_pos == std::string::npos) {
				//向发送发发送错误消息
				async_send("Invalid private message format. Use @username message\n");
				return;
			}
			std::string target_name = msg.substr(1, space_pos - 1);
			std::string context = msg.substr(space_pos+1);
			std::string private_msg = "[Private][" + target_name + "]" + context;
			if (server_.send_private_msg(target_name, private_msg, shared_from_this())) {
				//async_send(private_msg + "\n");  // 向发送者回显
				std::cout << "保存历史信息中" << std::endl;
				server_.save_history_msg(get_name(), target_name, context);
				std::cout << "保存历史信息成功" << std::endl;

				std::cout << "msg sended" << std::endl;
			}
			else
				async_send("User not found! Private chat failed.\n");
		}
		//发送添加好友消息
		else if (msg.substr(0, 10) == "ADDFRIEND|") {
			size_t pos = msg.find('|');
			if (pos == std::string::npos) {
				async_send("Invalid adding friends format. Use ADDFRIEND| message\n");
				return;
			}
			std::string target_name = msg.substr(pos + 1);
			if (server_.check_user(target_name, shared_from_this())) {
				std::string f_msg = "Adding_friends_success|";
				async_send( f_msg+ target_name);  // 向发送者回显
				std::string sender_name = get_name();
				server_.send_private_msg(target_name,f_msg+sender_name,shared_from_this());
				std::cout << "msg checking success" << std::endl;
			}
				
			else
				async_send("User not found!\n");
		}
		//获取历史消息
		else if (msg.substr(0,8) == "History|") {
			std::cout << "开始查询历史消息" << std::endl;
			/*size_t pos = msg.find('|', 8);
			if (pos == std::string::npos) {
				std::cout << "查询历史消息格式有误" << std::endl;
				return;
			}*/
			std::string target_name = msg.substr(8);
			std::cout << "查询历史消息对象为"<<target_name << std::endl;
			auto history = server_.get_history_msgs(get_name(), target_name, 1);
			if (history.empty()) {
				std::cout << " 消息为空" << std::endl;
				return;
			}
			for (const auto& m : history) {
				std::string ms = "his|" + m;
				async_send(ms);
				std::cout << "消息发送成功" << std::endl;
			}
			std::cout << "结束" << std::endl;
		}
		else {
			//公聊消息
			std::string broadcast_msg = "[" + name_ + "] " + msg;
			server_.broadcast_public_msg(broadcast_msg, shared_from_this());
		}
	}
}
