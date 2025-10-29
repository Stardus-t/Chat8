#include "GateGrpcClient.h"
#include "const.h"

UserUidRsp GateGrpcClient::GetUseruid(bool is_chat,std::string user_name,std::string target_name) {
	ClientContext context;
	UserUidReq request;
	UserUidRsp response;
	request.set_is_chat(is_chat);
	request.set_user_name(target_name);
	request.set_sender_uid(user_name);
	auto stub = pool_->getConnection();
	Status status = stub->GetUseruid(&context, request, &response);
	Defer defer([&stub, this]() {
		pool_->returnConnection(std::move(stub));
	});
	if (status.ok()) return response;
	else {
		response.set_error(ErrorCodes::RPCFailed);
		return response;
	}
}

GateGrpcClient::GateGrpcClient() {
	std::string GateService_host = ConfigMgr::Inst().GetValue("GateServer", "Host");
	std::string GateService_port = ConfigMgr::Inst().GetValue("GateServer", "Grpc_Port");
	pool_.reset(new GateConPool(5, GateService_host,GateService_port));
	
}