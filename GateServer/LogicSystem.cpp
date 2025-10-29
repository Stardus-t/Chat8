#include "LogicSystem.h"
#include "Singleton.h"
#include <functional>
#include <map>
#include "const.h"
#include "HttpConnection.h"
#include <iostream>
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"

void LogicSystem::RegGet(std::string url, HttpHandler handler) {
	_get_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem() {
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req " << std::endl;
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
	});
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
		std::cout << "email is " << email << std::endl;
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});
	//RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
	//	auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
	//	std::cout << "receive body is " << body_str << std::endl;
	//	connection->_response.set(http::field::content_type, "text/json");
	//	Json::Value root;
	//	Json::Reader reader;
	//	Json::Value src_root;
	//	bool parse_success = reader.parse(body_str, src_root);
	//	if (!parse_success) {
	//		std::cout << "Failed to parse JSON data!" << std::endl;
	//		root["error"] = ErrorCodes::Error_Json;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}
	//	//�ȿ�����֤���Ƿ���ȷ
	//	std::string varify_code;
	//	std::string key = CODEPREFIX + src_root["email"].asString();
	//	bool b_get_varify = RedisMgr::GetInstance()->Get(key, varify_code);//��Redis�л�ȡ��֤��
	//	if (!b_get_varify) {//�����ȡ��֤��ʧ��
	//		std::cout << "get varify code from redis failed" << std::endl;
	//		root["error"] = ErrorCodes::VarifyExpired;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}
	//	if (varify_code != src_root["varifycode"].asString()) {//�����֤�벻ƥ��
	//		std::cout << "varify code is not match" << std::endl;
	//		root["error"] = ErrorCodes::VarifyExpired;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}
	//	//����redis
	//	bool b_user_exists = RedisMgr::GetInstance()->ExistsKey(src_root["email"].asString());	
	//	if (b_user_exists) {//����û��Ѿ�����
	//		std::cout << "user already exists" << std::endl;
	//		root["error"] = ErrorCodes::VarifyExpired;
	//		std::string jsonstr = root.toStyledString();
	//		beast::ostream(connection->_response.body()) << jsonstr;
	//		return true;
	//	}
	//	//�������ݿ��ж��û��Ƿ����
	//	root["error"] = 0;
	//	root["email"] = src_root["email"];
	//	root["username"] = src_root["username"].asString();
	//	root["password"] = src_root["password"].asString();
	//	root["confirm"] = src_root["confirm"].asString();
	//	root["varify_code"] = src_root["varify_code"].asString();
	//	std::string jsonstr = root.toStyledString();
	//	beast::ostream(connection->_response.body()) << jsonstr;
	//	return true;
	//});
	//day11 ע���û��߼�
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["password"].asString();
		auto confirm = src_root["confirm"].asString();
		auto icon = src_root["icon"].asString();

		if (pwd != confirm) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//�Ȳ���redis��email��Ӧ����֤���Ƿ����
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//�������ݿ��ж��û��Ƿ����
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd, icon);
		if (uid == 0 || uid == -1) {
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		root["error"] = 0;
		root["uid"] = uid;
		root["email"] = email;
		root["user"] = name;
		root["password"] = pwd;
		root["confirm"] = confirm;
		root["icon"] = icon;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});

	//���ûص��߼�
	RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["password"].asString();

		//�Ȳ���redis��email��Ӧ����֤���Ƿ����
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//��ѯ���ݿ��ж��û����������Ƿ�ƥ��
		bool email_valid = MysqlMgr::GetInstance()->CheckEmail(name, email);
		if (!email_valid) {
			std::cout << " user email not match" << std::endl;
			root["error"] = ErrorCodes::EmailNotMatch;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//��������Ϊ��������
		bool b_up = MysqlMgr::GetInstance()->UpdatePwd(name, pwd);
		if (!b_up) {
			std::cout << " update pwd failed" << std::endl;
			root["error"] = ErrorCodes::PasswdUpFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		std::cout << "succeed to update password" << pwd << std::endl;
		root["error"] = 0;
		root["email"] = email;
		root["user"] = name;
		root["password"] = pwd;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});
	//�û���¼�߼�
	RegPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto name = src_root["user"].asString();
		auto pwd = src_root["password"].asString();
		UserInfo userInfo;
		//��ѯ���ݿ��ж��û����������Ƿ�ƥ��
		bool pwd_valid = MysqlMgr::GetInstance()->CheckPwd(name, pwd, userInfo);
		if (!pwd_valid) {
			std::cout << " user pwd not match" << std::endl;
			root["error"] = ErrorCodes::PasswdInvalid;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//�����û��ĺ����б�
		std::vector<UserInfo> friends;
		bool getFriendSuccess = MysqlMgr::GetInstance()->GetFriend(userInfo.uid, friends);
		if (!getFriendSuccess) {
			std::cout << "���Ҹ��û��ĺ���ʧ��" << std::endl;
			root["error"] = ErrorCodes::GetFriendFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		std::cout << "���Ҹ��û��ĺ��ѳɹ�" << std::endl;

		//std::cout << "�����˻�UidΪ" << userInfo.uid << std::endl;
		//��ѯStatusServer�ҵ����ʵ�����
		auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
		if (reply.error()) {
			std::cout << " grpc get chat server failed, error is " << reply.error() << std::endl;
			root["error"] = ErrorCodes::RPCGetFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;

		// ��ӡChatServer������Ϣ
		std::cout << "��ȡ����ChatServer����: " << std::endl;
		std::cout << "Host: " << reply.host() << std::endl;
		std::cout << "Port: " << reply.port() << std::endl;
		std::cout << "Token: " << reply.token() << std::endl;

		root["error"] = 0;
		root["user"] = name;
		root["uid"] = userInfo.uid;
		root["token"] = reply.token();
		root["host"] = reply.host();
		root["port"] = reply.port();
		Json::Value friendsArray;
		for (const auto& friendInfo : friends) {
			Json::Value friendObj;
			friendObj["uid"] = friendInfo.uid;
			friendObj["name"] = friendInfo.name;
			friendsArray.append(friendObj);
		}
		root["friends"] = friendsArray;
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});
}

void LogicSystem::RegPost(std::string url, HttpHandler handler) {
	_post_handlers.insert(make_pair(url, handler));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con) {
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}

	_post_handlers[path](con);
	return true;
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) {
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}

	_get_handlers[path](con);
	return true;
}



