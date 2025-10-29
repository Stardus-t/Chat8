#pragma once
#include <string>
#include <functional>

extern int current_connection;
extern std::string service_key;

extern std::string current_server;

class global
{
	global();
};

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
};

