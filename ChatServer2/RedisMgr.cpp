#include "RedisMgr.h"
#include <iostream>
#include <winsock.h>

bool RedisMgr::Init(const std::string& host, int port, const std::string passwd)
{
	Close();//关闭已经存在的链接
	//链接Redis服务器
	struct timeval timeout = { 1, 500000 };//1.5秒超时
	ctx_ = redisConnectWithTimeout(host.c_str(), port, timeout);
	if (ctx_ == nullptr || ctx_->err != 0) {
		std::cerr<<"Redis connect failed: "<< (ctx_ ? ctx_->errstr : "malloc error") << std::endl;
		return false;
	}
	if(!passwd.empty()) {
		//验证密码
		redisReply* reply = (redisReply*)redisCommand(ctx_,"AUTH %s",passwd.c_str());
		if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
			std::cerr << "Redis auth failed: " << (reply ? reply->str : "unknown error") << std::endl;
			freeReplyObject(reply);
			redisFree(ctx_);
			ctx_ = nullptr;
			return false;
		}
		freeReplyObject(reply);
	}  
	std::string port_str = std::to_string(port);
	std::cout << "Redis connected:host:"<<host.c_str()<<":port:" << port_str << std::endl;
	return true;
}

void RedisMgr::Close(){
	if (ctx_) {
		redisFree(ctx_);
		ctx_ = nullptr;
	}
}

bool RedisMgr::HSet(const std::string& key, const std::string* field, const std::string& value){
	if (ctx_ == nullptr || field == nullptr) return false;
	redisReply* reply = (redisReply*)redisCommand(ctx_, "HSET %s %s %s", key.c_str(), field->c_str(),value.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "HSet failed: " << (reply ? reply->str:"null reply") << std::endl;
		return false;
	}
	freeReplyObject(reply);
	return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& field){
	if (ctx_ == nullptr) return "";
	redisReply* reply = (redisReply*)redisCommand(ctx_, "HGET %s %s", key.c_str(), field.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "HGet failed: " << (reply ? reply->str : "null reply") << std::endl;
		return "";
	}
	std::string value;
	if (reply->type == REDIS_REPLY_STRING) value = reply->str;
	else if (reply->type == REDIS_REPLY_NIL) value = "";
	else std::cerr << "HGet unexpected reply type: " << reply->type << std::endl;
	freeReplyObject(reply);
	return value;
}

bool RedisMgr::HDel(const std::string& key, const std::string& field) {
	if (ctx_ == nullptr) return "";
	redisReply* reply = (redisReply*)redisCommand(ctx_, "HDEL %s %s", key.c_str(), field.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "HDEL Failed: " << (reply ? reply->str : "null reply") << std::endl;
		return false;
	}
	bool success = (reply->integer > 0);
	freeReplyObject(reply);
	return success;
}

bool RedisMgr::Set(const std::string &key, const std::string &value) {
	if (ctx_ == nullptr) return false;
	redisReply* reply = (redisReply*)redisCommand(ctx_, "SET %s %s", key.c_str(), value.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "Set Failed: " << (reply ? reply->str : "null reply") << std::endl;
		return false;
	}
	freeReplyObject(reply);
	return true;
}

std::string RedisMgr::Get(const std::string &key) {
	if (ctx_ == nullptr) return "";
	redisReply* reply = (redisReply*)redisCommand(ctx_,"Get %s",key.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "Get Failed: " << (reply ? reply->str : "null reply");
		return "";
	}
	std::string value;
	if (reply->type == REDIS_REPLY_STRING) value = reply->str;
	else if (reply->type == REDIS_REPLY_NIL) value = "";
	else std::cerr << "Get an unexpected type: " << reply->type << std::endl;
	freeReplyObject(reply);
	return value;
}

bool RedisMgr::Del(const std::string& key) {
	if (ctx_ == nullptr) return false;
	redisReply* reply = (redisReply*)redisCommand(ctx_, "DEL %s", key.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "Del Failed: " << (reply ? reply->str : "null reply");
		return false;
	}
	bool success = (reply->integer > 0);
	freeReplyObject(reply);
	return success;
}

bool RedisMgr::ZAdd(const std::string& key, long long score, const std::string& value) {
	if (ctx_ == nullptr) return false;
	redisReply* reply = (redisReply*)redisCommand(ctx_, "ZADD %s %lld %s", key.c_str(), score, value.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "ZAdd failed: " << (reply ? reply->str : "null reply") << std::endl;
		freeReplyObject(reply); // 注意释放reply
		return false;
	}
	freeReplyObject(reply);
	return true;
}

std::vector<std::string> RedisMgr::ZRange(const std::string& key, int start, int stop) {
	std::vector<std::string> result;
	if (ctx_ == nullptr) {
		std::cerr << "Redis context is null" << std::endl;
		return result;
	}

	redisReply* reply = (redisReply*)redisCommand(ctx_, "ZRANGE %s %d %d",
		key.c_str(), start, stop);

	if (reply == nullptr) {
		std::cerr << "ZRange failed: null reply" << std::endl;
		return result;
	}

	if (reply->type == REDIS_REPLY_ERROR) {
		std::cerr << "ZRange error: " << reply->str << std::endl;
		freeReplyObject(reply);
		return result;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		// 遍历数组元素，添加到结果列表
		for (size_t i = 0; i < reply->elements; ++i) {
			if (reply->element[i]->type == REDIS_REPLY_STRING) {
				result.push_back(reply->element[i]->str);
			}
		}
	}
	else {
		std::cerr << "ZRange unexpected reply type: " << reply->type << std::endl;
	}

	freeReplyObject(reply);
	return result;
}