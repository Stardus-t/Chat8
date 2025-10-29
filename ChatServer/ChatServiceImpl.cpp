#include "ChatServiceImpl.h"


Status ChatServiceImpl::GetReplyMessage(::grpc::ServerContext* context, const ::message::ChatingReq* request, ::message::ChatingRsp* response) {
    std::cout << "�õ��ظ�" << std::endl;
	int sendUid = request->sender_uid();
	std::string senderName = request->sender_name();
	int recvUid = request->recv_uid();
	std::string recvName = request->recv_name();
	std::string content = request->content();
	/*std::cout << "�������յ���Ϣ - ������[" << sendUid << ":" << senderName
		<< "] ������[" << recvUid << ":" << recvName
		<< "] ����: " << content << std::endl;*/

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
        response->set_error_msg("�����߲����ڻ�������");
    }
    return Status::OK;
}

//ChatServiceImpl::ChatServiceImpl() {}
