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
	// 接受一个lambda表达式或者函数指针
	Defer(std::function<void()> func) : func_(func) {}

	// 析构函数中执行传入的函数
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json解析错误
	RPCFailed = 1002,  //RPC请求错误
	VarifyExpired = 1003, //验证码过期
	PasswdErr = 1004, //密码错误
	VarifyCodeErr = 1005, //验证码错误
	UserExist = 1006, //用户已存在
	EmailNotMatch = 1007, //邮箱不匹配
	PasswdUpFailed = 1008, //密码更新失败
	PasswdInvalid = 1009, //密码无效
	RPCGetFailed = 1010, //RPC获取失败
};

