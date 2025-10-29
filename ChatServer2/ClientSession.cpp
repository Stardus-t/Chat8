#include "ClientSession.h"
#include "TcpChatServer.h"
#include <fstream>
#include "ConfigMgr.h"

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
			bool is_chat = true;
			if (server_.send_private_msg(is_chat,name_,target_name, private_msg, shared_from_this())) {
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
			std::cout << "添加好友信息时的当前用户名为" << name_ << std::endl;
			bool is_chat = false;
			if (server_.check_user(is_chat,name_,target_name, shared_from_this())) {
				std::string f_msg = "Adding_friends_success|";
				async_send( f_msg+ target_name);  // 向发送者回显
				std::string sender_name = get_name();
				is_chat = true;
				server_.send_private_msg(is_chat,name_,target_name,f_msg+sender_name,shared_from_this());
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
		else if (msg.substr(0, 15) == "SEARCH_HISTORY|") {
			std::string sub_str1 = msg.substr(15);
			int pos_friend_name = sub_str1.find('|');
			std::string friend_name = sub_str1.substr(0, pos_friend_name);
			std::string msg = sub_str1.substr(pos_friend_name + 1);
			std::cout << "收到搜索历史消息,好友名为：" << friend_name << ",消息内容为：" << msg << std::endl;
			auto history = server_.get_history_msgs(get_name(), friend_name, 1);
			if (history.empty()) {
				std::cout << " 消息为空" << std::endl;
				return;
			}
			std::string history_msg = "历史消息如下:\n";
			for (const auto& his : history)
				history_msg += his + "\n";
			history_msg += msg;

			//将history_msg写入到一个临时文件中
			std::string temp_file = ConfigMgr::GetInstance()->GetValue("LLA", "Temp_file");
			std::ofstream fout(temp_file);
			if (!fout) {
				std::cerr << "创建临时文件失败" << std::endl;
				return;
			}
			fout << history_msg;
			fout.close();
			//传入python 处理
			std::string python_exe = ConfigMgr::GetInstance()->GetValue("LLA", "Python_exe");
			std::string script_path = ConfigMgr::GetInstance()->GetValue("LLA", "Script_path");
			std::string python_cmd = python_exe + " " + script_path+" "+temp_file;
			FILE* read_pipe = _popen(python_cmd.c_str(), "r");  // 仅读取模式
			if (!read_pipe) {
				std::cerr << "读取Python输出失败" << std::endl;
				return;
			}

			//FILE* pipe = _popen(python_cmd.c_str(), "r+");
			//if (!pipe) {
			//	std::cerr << "调用Python脚本失败" << std::endl;
			//	return;
			//}
			//fwrite(history_msg.c_str(), 1, history_msg.size(), pipe);
			//fflush(pipe);
			//fclose(pipe->_file);
			char buffer[1024];
			std::string result;
			while (fgets(buffer, sizeof(buffer), read_pipe) != nullptr) {
				result += buffer;
			}
			_pclose(read_pipe);

			// 处理结果（例如发送给客户端）
			std::cout << "Python处理结果: " << result << std::endl;
			std::string sended_msg = "SEARCH_RESULT|" + result;
			async_send(sended_msg);
			std::cout << "消息发送成功:" <<std::endl<<sended_msg<< std::endl;
		}
		else {
			//公聊消息
			std::string broadcast_msg = "[" + name_ + "] " + msg;
			server_.broadcast_public_msg(broadcast_msg, shared_from_this());
		}
	}
}
