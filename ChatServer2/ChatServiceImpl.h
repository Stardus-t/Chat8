#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>
#include "message.pb.h"
#include <string>
#include <unordered_map>
#include "TcpChatServer.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::ChatingReq;
using message::ChatingRsp;
using message::ChatService;

class ChatServiceImpl final:public ChatService::Service
{
public:
	explicit ChatServiceImpl(TcpChatServer& server) : server_(server) {}
	Status GetReplyMessage(::grpc::ServerContext* context, const ::message::ChatingReq* request, ::message::ChatingRsp* response);
	
private:
	TcpChatServer& server_;
};

