#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>
#include "message.pb.h"   
#include <string>
#include <unordered_map>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::UserUidReq;
using message::UserUidRsp;
using message::GateService;

class GateServiceImpl final :public GateService::Service 
{
public:
	GateServiceImpl();
	Status GetUseruid(ServerContext *context,const UserUidReq *request,UserUidRsp *reply);
};

