#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
class ConfigMgr;
extern ConfigMgr gCfgMgr;

struct UserInfo {
	int uid;         // �û�ΨһID
	std::string name; // �û���
	std::string pwd;  // ���루���ݿ��д洢�ļ���/ԭʼ���룩
	std::string email;// �����ַ
};


enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json��������
	RPCFailed = 1002,  //RPC�������
	VarifyExpired = 1003, //��֤�����
	PasswdErr = 1004, //�������
	VarifyCodeErr = 1005, //��֤�����
	UserExist = 1006, //�û��Ѵ���
	EmailNotMatch = 1007, //���䲻ƥ��
	PasswdUpFailed = 1008, //�������ʧ��
	PasswdInvalid = 1009, //������Ч
	RPCGetFailed = 1010, //RPC��ȡʧ��
	GetFriendFailed = 1011, //��ȡ����ʧ��
};


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;

class Defer {
public:
	// ����һ��lambda���ʽ���ߺ���ָ��
	Defer(std::function<void()> func) : func_(func) {}

	// ����������ִ�д���ĺ���
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};

#define CODEPREFIX  "code_"