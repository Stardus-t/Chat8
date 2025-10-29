#pragma once
#include <memory>
#include <string>
#include "jdbc\mysql_driver.h"
#include "jdbc\mysql_connection.h"

class MySqlConnection {
public:
    // 构造函数：初始化单个数据库连接
    MySqlConnection(const std::string& url, const std::string& user,
        const std::string& pass, const std::string& schema)
        : url_(url), user_(user), pass_(pass), schema_(schema) {
        try {
            // 创建驱动并建立连接
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            conn_.reset(driver->connect(url_, user_, pass_));
            conn_->setSchema(schema_); // 设置数据库
        }
        catch (sql::SQLException& e) {
            std::cerr << "MySQL连接初始化失败: " << e.what() << std::endl;
            std::cerr << "错误码: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << std::endl;
        }
    }

    // 析构函数：自动释放连接
    ~MySqlConnection() = default;

    // 获取当前连接（非并发场景直接返回）
    sql::Connection* getConn() {
        return conn_.get();
    }

private:
    std::string url_;       // 连接地址
    std::string user_;      // 用户名
    std::string pass_;      // 密码
    std::string schema_;    // 数据库名
    std::unique_ptr<sql::Connection> conn_; // 单个数据库连接
};