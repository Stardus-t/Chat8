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
		// ����������һЩ�߽���  
		return _section_datas[key];
	}
};
class ConfigMgr
{
public:
	static ConfigMgr& Inst() {
		static ConfigMgr instance; // ��̬�ֲ���������֤ȫ��Ψһ
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

	ConfigMgr(const ConfigMgr& src) = delete; // ɾ����������
	ConfigMgr& operator=(const ConfigMgr& src) = delete;

	/*ConfigMgr();*/
	ConfigMgr() {
		// �˴��ɴ������ļ��������ݣ�ʾ���ֶ���ʼ��MySQL����
		SectionInfo mysqlSection;
		mysqlSection._section_datas["Host"] = "localhost";
		mysqlSection._section_datas["Port"] = "3308";
		mysqlSection._section_datas["User"] = "root";
		mysqlSection._section_datas["Passwd"] = "123456";
		mysqlSection._section_datas["Schema"] = "llfc";
		_config_map["Mysql"] = mysqlSection;

		// ��ȡ��ǰ����Ŀ¼  
		boost::filesystem::path current_path = boost::filesystem::current_path();
		// ����config.ini�ļ�������·��  
		boost::filesystem::path config_path = current_path / "config.ini";
		std::cout << "Config path: " << config_path << std::endl;

		// ʹ��Boost.PropertyTree����ȡINI�ļ�  
		boost::property_tree::ptree pt;
		boost::property_tree::read_ini(config_path.string(), pt);


		// ����INI�ļ��е�����section  
		for (const auto& section_pair : pt) {
			const std::string& section_name = section_pair.first;
			const boost::property_tree::ptree& section_tree = section_pair.second;

			// ����ÿ��section�����������е�key-value��  
			std::map<std::string, std::string> section_config;
			for (const auto& key_value_pair : section_tree) {
				const std::string& key = key_value_pair.first;
				const std::string& value = key_value_pair.second.get_value<std::string>();
				section_config[key] = value;
			}
			SectionInfo sectionInfo;
			sectionInfo._section_datas = section_config;
			// ��section��key-value�Ա��浽config_map��  
			_config_map[section_name] = sectionInfo;
		}

		// ������е�section��key-value��  
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
	
	// �洢section��key-value�Ե�map  
	std::map<std::string, SectionInfo> _config_map;
};

