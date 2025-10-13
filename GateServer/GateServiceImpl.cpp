#include "GateServiceImpl.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include <string>

GateServiceImpl::GateServiceImpl() {
	auto& cfg = ConfigMgr::Inst();
}

Status GateServiceImpl::GetUseruid(ServerContext* context, const UserUidReq* request, UserUidRsp* reply) {
	std::string target_name = request->user_name();
	int target_uid;
	std::cout << "µÃµ½ÇëÇó" << std::endl;
	if (MysqlMgr::GetInstance()->GetUid(target_name, target_uid)) {
		std::string target_uid_str = std::to_string(target_uid);
		std::cout<<"target uid is"<<target_uid<<std::endl;
		reply -> set_target_uid(target_uid_str);
		reply->set_error(ErrorCodes::Success);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::RPCGetFailed);
	return Status::OK;
}