#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include <memory>
#include "Singleton.h"
#include <vector>

class RedisMgr:public Singleton<RedisMgr>{
public:
	bool Init(const std::string& host,int port,const std::string passwd);
	void Close();
	//存储连接数
	bool HSet(const std::string& key,const std::string*field,const std::string& value);
	std::string HGet(const std::string& key,const std::string& field);
	bool HDel(const std::string& key, const std::string& field);

	//存储用户信息
	bool Set(const std::string& key, const std::string& value);
	std::string Get(const std::string& key);
	bool Del(const std::string& key);
	bool ZAdd(const std::string &key, long long score,const std::string& value);
	std::vector<std::string> ZRange(const std::string& key, int start, int stop);
private:
	redisContext* ctx_ = nullptr;
};

