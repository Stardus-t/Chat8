#include "StatusGrpcClient.h"
#include "global.h"

UserServerRsp StatusGrpcClient::GetUserServer(int uid) {
	ClientContext context;
	UserServerReq request;
	UserServerRsp response;
	std::string target_uid = std::to_string(uid);
	request.set_target_uid(target_uid);
	auto stub = pool_->getConnection();
	Status status = stub->GetUserServer(&context, request, &response);
	Defer defer([&stub, this]() {
		pool_->returnConnection(std::move(stub));
	});
	if (status.ok()) return response;
	else {
		response.set_error(ErrorCodes::RPCGetFailed);
		return response;
	}
}

UserUidRsp StatusGrpcClient::GetUserUid(std::string target_name) {
	ClientContext context;
	UserUidReq request;
	UserUidRsp response;
	request.set_user_name(target_name);
	auto stub = pool_->getConnection();
	Status status = stub->GetUserUid(&context, request, &response);
	Defer defer([&stub, this]() {
		pool_->returnConnection(std::move(stub));
		});
	if (status.ok())return response;
	else {
		response.set_error(ErrorCodes::RPCFailed);
	}
}

StatusGrpcClient::StatusGrpcClient() {
	std::string StatusServer_host=ConfigMgr::GetInstance()->GetValue("StatusServer","Host");
	std::string StatusServer_port = ConfigMgr::GetInstance()->GetValue("StatusServer", "Port");
	pool_.reset(new StatusConPool(5, StatusServer_host, StatusServer_port));
	if (StatusServer_host.empty() || StatusServer_port.empty()) {
		// 日志报错或抛出异常，避免使用无效地址
		throw std::runtime_error("Invalid StatusServer host or port");
	}

}
