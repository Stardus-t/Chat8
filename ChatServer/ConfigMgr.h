#pragma once
#include <map>
#include <string>
#include "Singleton.h"

class ConfigMgr:public Singleton<ConfigMgr>{
public:
	bool Load(const std::string &path);
	std::string GetValue(const std::string& section,const std::string& key);
private:
	std::map<std::string, std::map<std::string, std::string>> configs_;
};

