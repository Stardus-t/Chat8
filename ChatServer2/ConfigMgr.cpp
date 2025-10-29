#include "ConfigMgr.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

bool ConfigMgr::Load(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Config file open failed: " << path << std::endl;
		return false;
	}
	std::cerr << "Config file loaded: " << path << std::endl;
	std::string line, currentSection;
	while (std::getline(file,line)) {
		//È¥³ýÊ×Î²¿Õ¸ñ
		//line.erase(0, line.find_first_not_of(" "));
		//line.erase(0, line.find_last_not_of(" ")+1);
		if (line.empty() || line[0] == ';') continue; //Ìø¹ý¿ÕÐÐºÍ×¢ÊÍ
		//½âÎösection
		if (line[0] == '[' && line.back() == ']') {
			currentSection = line.substr(1, line.size() - 2);
			std::cerr << "Section found: " << currentSection << std::endl;
			continue;
		}
		//½âÎökey = value
		size_t eq_pos = line.find('=');
		if (eq_pos == std::string::npos) continue;
		std::string key = line.substr(0, eq_pos);
		std::string value = line.substr(eq_pos + 1);
		key.erase(key.find_last_not_of(" \t") + 1); // ÐÞ¼ôkey
		value.erase(0, value.find_first_not_of(" \t")); // ÐÞ¼ôvalue
		configs_[currentSection][key] = value;
	}
	return true;
}

std::string ConfigMgr::GetValue(const std::string& section, const std::string& key) {
	auto it = configs_.find(section);
	if (it != configs_.end()) {
		auto key_it = it->second.find(key);
		if (key_it != it->second.end()) {
			return key_it->second;
			std::cout << "Config found: [" << section << "] " << key << " = " << key_it->second << std::endl;
		}
	}
	return "";
}