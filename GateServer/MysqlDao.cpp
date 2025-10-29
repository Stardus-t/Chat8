#include "MysqlDao.h"
#include "ConfigMgr.h"
#include <memory>
#include "jdbc\cppconn\statement.h"
#include "jdbc\cppconn\prepared_statement.h"
#include <memory>
#include <iostream>

MysqlDao::MysqlDao()
{
	auto& cfg = ConfigMgr::Inst();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() {
	pool_->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		// ׼�����ô洢����
		std::unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// �����������
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, pwd);

		// ����PreparedStatement��ֱ��֧��ע�����������������Ҫʹ�ûỰ������������������ȡ���������ֵ

		  // ִ�д洢����
		stmt->execute();
		// ����洢���������˻Ự��������������ʽ��ȡ���������ֵ�������������ִ��SELECT��ѯ����ȡ����
	   // ���磬����洢����������һ���Ự����@result���洢������������������ȡ��
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		if (res->next()) {
			int result = res->getInt("result");
			std::cout << "Result: " << result << std::endl;
			pool_->returnConnection(std::move(con));
			return result;
		}
		pool_->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

int MysqlDao::RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd,
	const std::string& icon)
{
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con] {
		pool_->returnConnection(std::move(con));
		});

	try {
		//��ʼ����
		con->_con->setAutoCommit(false);
		//ִ�е�һ�����ݿ����������email�����û�
			// ׼����ѯ���

		std::unique_ptr<sql::PreparedStatement> pstmt_email(con->_con->prepareStatement("SELECT 1 FROM user WHERE email = ?"));

		// �󶨲���
		pstmt_email->setString(1, email);

		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res_email(pstmt_email->executeQuery());

		auto email_exist = res_email->next();
		if (email_exist) {
			con->_con->rollback();
			std::cout << "email " << email << " exist";
			return 0;
		}

		// ׼����ѯ�û����Ƿ��ظ�
		std::unique_ptr<sql::PreparedStatement> pstmt_name(con->_con->prepareStatement("SELECT 1 FROM user WHERE name = ?"));

		// �󶨲���
		pstmt_name->setString(1, name);

		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res_name(pstmt_name->executeQuery());

		auto name_exist = res_name->next();
		if (name_exist) {
			con->_con->rollback();
			std::cout << "name " << name << " exist";
			return 0;
		}

		// ׼�������û�id
		std::unique_ptr<sql::PreparedStatement> pstmt_upid(con->_con->prepareStatement("UPDATE user_id SET id = id + 1"));

		// ִ�и���
		pstmt_upid->executeUpdate();

		// ��ȡ���º�� id ֵ
		std::unique_ptr<sql::PreparedStatement> pstmt_uid(con->_con->prepareStatement("SELECT id FROM user_id"));
		std::unique_ptr<sql::ResultSet> res_uid(pstmt_uid->executeQuery());
		int newId = 0;
		// ��������
		if (res_uid->next()) {
			newId = res_uid->getInt("id");
		}
		else {
			std::cout << "select id from user_id failed" << std::endl;
			con->_con->rollback();
			return -1;
		}

		// ����user��Ϣ
		std::unique_ptr<sql::PreparedStatement> pstmt_insert(con->_con->prepareStatement("INSERT INTO user (uid, name, email, pwd, nick, icon) "
			"VALUES (?, ?, ?, ?,?,?)"));
		pstmt_insert->setInt(1, newId);
		pstmt_insert->setString(2, name);
		pstmt_insert->setString(3, email);
		pstmt_insert->setString(4, pwd);
		pstmt_insert->setString(5, name);
		pstmt_insert->setString(6, icon);
		//ִ�в���
		pstmt_insert->executeUpdate();
		// �ύ����
		con->_con->commit();
		std::cout << "newuser insert into user success" << std::endl;
		return newId;
	}
	catch (sql::SQLException& e) {
		// ����������󣬻ع�����
		if (con) {
			con->_con->rollback();
		}
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		// ׼����ѯ���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));

		// �󶨲���
		pstmt->setString(1, name);

		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// ���������
		while (res->next()) {
			std::cout << "Check Email: " << res->getString("email") << std::endl;
			if (email != res->getString("email")) {
				pool_->returnConnection(std::move(con));
				return false;
			}
			pool_->returnConnection(std::move(con));
			return true;
		}
		return false;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		// ׼����ѯ���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

		// �󶨲���
		pstmt->setString(2, name);
		pstmt->setString(1, newpwd);

		// ִ�и���
		int updateCount = pstmt->executeUpdate();

		std::cout << "Updated rows: " << updateCount << std::endl;
		pool_->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {


		// ׼��SQL���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
		pstmt->setString(1, email); // ��username�滻Ϊ��Ҫ��ѯ���û���

		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		// ���������
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// �����ѯ��������
			std::cout << "Password: " << origin_pwd << std::endl;
			break;
		}

		if (pwd != origin_pwd) {
			return false;
		}
		std::cout << "�������룺" << pwd << std::endl << "��ѯ���룺" << origin_pwd << std::endl;
		userInfo.name = res->getString("name");
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
		
		userInfo.pwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::GetUid(const std::string& email, int& uid) {
	auto con = pool_->getConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});
	
	try {
		std::cout << "email: " << email << std::endl;
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
		pstmt->setString(1, email); 

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";

		if (res->next()) {
			uid = res->getInt("uid");
			return true;
		}
		else {
			return false;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::TestProcedure(const std::string& email, int& uid, std::string& name) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		Defer defer([this, &con]() {
			pool_->returnConnection(std::move(con));
			});
		// ׼�����ô洢����
		std::unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL test_procedure(?,@userId,@userName)"));
		// �����������
		stmt->setString(1, email);

		// ����PreparedStatement��ֱ��֧��ע�����������������Ҫʹ�ûỰ������������������ȡ���������ֵ

		  // ִ�д洢����
		stmt->execute();
		// ����洢���������˻Ự��������������ʽ��ȡ���������ֵ�������������ִ��SELECT��ѯ����ȡ����
	   // ���磬����洢����������һ���Ự����@result���洢������������������ȡ��
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @userId AS uid"));
		if (!(res->next())) {
			return false;
		}

		uid = res->getInt("uid");
		std::cout << "uid: " << uid << std::endl;

		stmtResult.reset(con->_con->createStatement());
		res.reset(stmtResult->executeQuery("SELECT @userName AS name"));
		if (!(res->next())) {
			return false;
		}

		name = res->getString("name");
		std::cout << "name: " << name << std::endl;
		return true;

	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::GetFriend(int& uid, std::vector<UserInfo>& friends) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		Defer defer([this, &con]() {
			pool_->returnConnection(std::move(con));
			});
		std::unique_ptr < sql::PreparedStatement > pstmt(con->_con->prepareStatement("SELECT friend_id From friend WHERE user_id=? AND status=1"));
		pstmt->setInt(1,uid);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		while (res->next())
		{
			int friend_id = res->getInt("friend_id");
			std::unique_ptr < sql::PreparedStatement > pstmt_friend(con->_con->prepareStatement("SELECT uid,email FROM user WHERE uid=?"));
			pstmt_friend->setInt(1, friend_id);
			std::unique_ptr<sql::ResultSet> res_user(pstmt_friend->executeQuery());
			if (res_user->next()) {
				UserInfo friendInfo;
				friendInfo.uid = res_user->getInt("uid");
				friendInfo.name = res_user->getString("email");
				friends.push_back(friendInfo);
			}
		}
		return true;

	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
	return true;
}

bool MysqlDao::loadFriend(std::string user_name, std::string target_name, int target_uid) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}

		Defer defer([this, &con]() {
			pool_->returnConnection(std::move(con));
			});
		std::cout << "���غ�����ʹ�õ��û�����" << user_name << "," << target_name << std::endl;
		std::unique_ptr < sql::PreparedStatement > pstmt1(con->_con->prepareStatement("SELECT uid From user WHERE email=?"));
		pstmt1->setString(1, user_name);
		std::unique_ptr<sql::ResultSet> res1(pstmt1->executeQuery());

		std::unique_ptr < sql::PreparedStatement > pstmt2(con->_con->prepareStatement("SELECT uid From user WHERE email=?"));
		pstmt2->setString(1, target_name);
		std::unique_ptr<sql::ResultSet> res2(pstmt2->executeQuery());

		if (res1->next()&&res2->next()) {
			int user_uid = res1->getInt("uid");
			int target_uid = res2->getInt("uid");
			std::unique_ptr<sql::PreparedStatement> pstmt3(
				con->_con->prepareStatement("INSERT INTO friend (user_id, friend_id, status) VALUES (?, ?, 1)")
			);
			pstmt3->setInt(1, user_uid);   
			pstmt3->setInt(2, target_uid); 
			//pstmt3->executeUpdate();
			std::cout << "pstmt3 affected rows: " << pstmt3->executeUpdate() << std::endl;
			std::unique_ptr<sql::PreparedStatement> pstmt4(
				con->_con->prepareStatement("INSERT INTO friend (user_id, friend_id, status) VALUES (?, ?, 1)")
			);
			pstmt4->setInt(1, target_uid);
			pstmt4->setInt(2, user_uid);
			//pstmt4->executeUpdate();
			std::cout << "pstmt4 affected rows: " << pstmt4->executeUpdate() << std::endl;
			return true;
		}
		return false;

	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}

}