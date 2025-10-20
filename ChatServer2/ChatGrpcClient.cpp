#include "ChatGrpcClient.h"
#include "global.h"

ChatingRsp ChatGrpcClient::GetReplyMessage(const std::string& target_host, const std::string& target_port, std::string sender_name, int sender_uid,
	std::string recv_name, int recv_uid, std::string contexts) {
	ClientContext context;
	ChatingReq request;
	ChatingRsp response;
	request.set_sender_name(sender_name);
	request.set_sender_uid(sender_uid);
	request.set_content(contexts);
	request.set_recv_name(recv_name);
	request.set_recv_uid(recv_uid);
	auto stub = pool_->getConnection(target_host,target_port);
	Defer defer([&stub,target_host, target_port,this]() {
		pool_->returnConnection(std::move(stub), target_host, target_port);
		});
	Status status = stub->GetReplyMessage(&context, request, &response);
	if (status.ok()) return response;
	else {
		response.set_error(ErrorCodes::RPCFailed);
		return response;
	}
}

ChatGrpcClient::ChatGrpcClient() {
	pool_.reset(new ChatConPool(poolSize));
}
