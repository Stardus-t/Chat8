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
	int uid;         // 用户唯一ID
	std::string name; // 用户名
	std::string pwd;  // 密码（数据库中存储的加密/原始密码）
	std::string email;// 邮箱地址
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
	GetFriendFailed = 1011, //获取好友失败
};


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;

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

#define CODEPREFIX  "code_"