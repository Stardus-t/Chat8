#include "GateServiceImpl.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include <string>

GateServiceImpl::GateServiceImpl() {
	auto& cfg = ConfigMgr::Inst();
}

Status GateServiceImpl::GetUseruid(ServerContext* context, const UserUidReq* request, UserUidRsp* reply) {
	bool is_chat = request->is_chat();
	std::string sender_name = request->sender_uid();
	std::string target_name = request->user_name();
	int target_uid;
	//std::cout << "得到请求" << std::endl;
	if (MysqlMgr::GetInstance()->GetUid(target_name, target_uid)) {
		std::string target_uid_str = std::to_string(target_uid);
		std::cout<<"target uid is"<<target_uid<<std::endl;
		//将好友信息存入数据库
		if (!is_chat) {
			if (MysqlMgr::GetInstance()->loadFriend(sender_name, target_name, target_uid)) std::cout << "好友信息录入成功" << std::endl;
			else std::cout << "好友信息录入失败" << std::endl;
		}
		reply -> set_target_uid(target_uid_str);
		reply->set_error(ErrorCodes::Success);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::RPCGetFailed);
	return Status::OK;
}