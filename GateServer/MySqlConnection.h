#pragma once
#include <memory>
#include <string>
#include "jdbc\mysql_driver.h"
#include "jdbc\mysql_connection.h"

class MySqlConnection {
public:
    // ���캯������ʼ���������ݿ�����
    MySqlConnection(const std::string& url, const std::string& user,
        const std::string& pass, const std::string& schema)
        : url_(url), user_(user), pass_(pass), schema_(schema) {
        try {
            // ������������������
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            conn_.reset(driver->connect(url_, user_, pass_));
            conn_->setSchema(schema_); // �������ݿ�
        }
        catch (sql::SQLException& e) {
            std::cerr << "MySQL���ӳ�ʼ��ʧ��: " << e.what() << std::endl;
            std::cerr << "������: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << std::endl;
        }
    }

    // �����������Զ��ͷ�����
    ~MySqlConnection() = default;

    // ��ȡ��ǰ���ӣ��ǲ�������ֱ�ӷ��أ�
    sql::Connection* getConn() {
        return conn_.get();
    }

private:
    std::string url_;       // ���ӵ�ַ
    std::string user_;      // �û���
    std::string pass_;      // ����
    std::string schema_;    // ���ݿ���
    std::unique_ptr<sql::Connection> conn_; // �������ݿ�����
};