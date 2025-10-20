#pragma once
#include <map>
#include <string>
#include <boost/filesystem.hpp>      
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>

struct SectionInfo {
	SectionInfo() {}
	~SectionInfo() {
		_section_datas.clear();
	}

	SectionInfo(const SectionInfo& src) {
		_section_datas = src._section_datas;
	}

	SectionInfo& operator = (const SectionInfo& src) {
		if (&src == this) {
			return *this;
		}

		this->_section_datas = src._section_datas;
	}

	std::map<std::string, std::string> _section_datas;
	std::string  operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";
		}
		// 这里可以添加一些边界检查  
		return _section_datas[key];
	}
};
class ConfigMgr
{
public:
	static ConfigMgr& Inst() {
		static ConfigMgr instance; // 静态局部变量，保证全局唯一
		return instance;
	}

	~ConfigMgr() {
		_config_map.clear();
	}
	SectionInfo operator[](const std::string& section) {
		if (_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[section];
	}

	/*ConfigMgr& operator=(const ConfigMgr& src) {
		if (&src == this) {
			return *this;
		}

		this->_config_map = src._config_map;
	};

	ConfigMgr(const ConfigMgr& src) {
		this->_config_map = src._config_map;
	}*/

	ConfigMgr(const ConfigMgr& src) = delete; // 删除拷贝构造
	ConfigMgr& operator=(const ConfigMgr& src) = delete;

	/*ConfigMgr();*/
	ConfigMgr() {
		// 此处可从配置文件加载数据，示例手动初始化MySQL配置
		SectionInfo mysqlSection;
		mysqlSection._section_datas["Host"] = "localhost";
		mysqlSection._section_datas["Port"] = "3308";
		mysqlSection._section_datas["User"] = "root";
		mysqlSection._section_datas["Passwd"] = "123456";
		mysqlSection._section_datas["Schema"] = "llfc";
		_config_map["Mysql"] = mysqlSection;

		// 获取当前工作目录  
		boost::filesystem::path current_path = boost::filesystem::current_path();
		// 构建config.ini文件的完整路径  
		boost::filesystem::path config_path = current_path / "config.ini";
		std::cout << "Config path: " << config_path << std::endl;

		// 使用Boost.PropertyTree来读取INI文件  
		boost::property_tree::ptree pt;
		boost::property_tree::read_ini(config_path.string(), pt);


		// 遍历INI文件中的所有section  
		for (const auto& section_pair : pt) {
			const std::string& section_name = section_pair.first;
			const boost::property_tree::ptree& section_tree = section_pair.second;

			// 对于每个section，遍历其所有的key-value对  
			std::map<std::string, std::string> section_config;
			for (const auto& key_value_pair : section_tree) {
				const std::string& key = key_value_pair.first;
				const std::string& value = key_value_pair.second.get_value<std::string>();
				section_config[key] = value;
			}
			SectionInfo sectionInfo;
			sectionInfo._section_datas = section_config;
			// 将section的key-value对保存到config_map中  
			_config_map[section_name] = sectionInfo;
		}

		// 输出所有的section和key-value对  
		for (const auto& section_entry : _config_map) {
			const std::string& section_name = section_entry.first;
			SectionInfo section_config = section_entry.second;
			std::cout << "[" << section_name << "]" << std::endl;
			for (const auto& key_value_pair : section_config._section_datas) {
				std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
			}
		}
	}
private:
	
	// 存储section和key-value对的map  
	std::map<std::string, SectionInfo> _config_map;
};

