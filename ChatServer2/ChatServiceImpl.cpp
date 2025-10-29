#include "ChatServiceImpl.h"


Status ChatServiceImpl::GetReplyMessage(::grpc::ServerContext* context, const ::message::ChatingReq* request, ::message::ChatingRsp* response) {
    std::cout << "得到回复" << std::endl;
	int sendUid = request->sender_uid();
	std::string senderName = request->sender_name();
	int recvUid = request->recv_uid();
	std::string recvName = request->recv_name();
	std::string content = request->content();
	/*std::cout << "服务器收到消息 - 发送者[" << sendUid << ":" << senderName
		<< "] 接收者[" << recvUid << ":" << recvName
		<< "] 内容: " << content << std::endl;*/

	/*std::string msg_to_send = "[" + senderName + "] " + content;*/
    std::string msg_to_send = content;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(server_.get_mutex()); 
        for (const auto& session : server_.get_sessions()) {  
            if (session->get_name() == recvName) {       
                session->async_send(msg_to_send); 
                found = true;
                response->set_error(0);
                return Status::OK;
            }
        }
    }

    if (!found) {
        response->set_error(1);
        response->set_error_msg("接收者不存在或已离线");
    }
    return Status::OK;
}

//ChatServiceImpl::ChatServiceImpl() {}
