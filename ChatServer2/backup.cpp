//#include  <iostream>
//#include <winsock2.h>
//#include <cstring>
//#include <vector>
//#include <sstream>
//
//const int MAX = 5;//最大连接数
//#pragma comment(lib,"ws2_32.lib")
//
//struct Client {
//	SOCKET sockfd;
//	std::string name;
//	Client(SOCKET s) :sockfd(s), name("unname") {}
//};
//
//void BrodCastClientList(const std::vector<Client>&clients) {
//	std::stringstream ss;
//	ss << "USERLIST";
//	for (const auto &client : clients)  ss << "|" << client.name;
//	std::string msg = ss.str();
//	for (const auto& client : clients) send(client.sockfd,msg.c_str(),msg.size(),0);
//}
//
//Client *findClientByName(std::vector<Client>& clients, const std::string& name) {
//	for (auto& client : clients) {
//		if (client.name == name) return &client;
//	}
//	return nullptr;
//}
//
//int main() {
//	//初始化Winsock库
//	WSADATA wsaData;
//	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//		std::cerr << "init_error" << std::endl;
//		return 1;
//	}
//
//	//创建监听套接字
//	SOCKET sockfd=socket(AF_INET, SOCK_STREAM, 0);
//	if (sockfd==INVALID_SOCKET) {
//		perror("socket error");
//		WSACleanup();
//		return -1;
//	}
//
//	//绑定地址和端口
//	sockaddr_in addr;
//	addr.sin_family = AF_INET;
//	addr.sin_addr.s_addr = htonl(INADDR_ANY);
//	addr.sin_port = htons(9999);
//
//	int ret = bind(sockfd, (SOCKADDR*)&addr, sizeof(addr));
//	if (ret == SOCKET_ERROR) {
//		std::cout << "bind error" << std::endl;
//		return -1;
//	}
//
//	//开始监听
//	ret=listen(sockfd, 5);
//	if (ret == SOCKET_ERROR) {
//		std::cout << "listen error" << std::endl;
//		return -1;
//	}
//	std::cout << "Server Activate,waiting for connection..." << std::endl;
//
//	std::vector<Client> clients;
//	while (true) {
//		//初始化读事件集合
//		fd_set readSet;
//		FD_ZERO(&readSet);//初始化一个文件描述符集合
//		//将监听套接字加入读事件集合
//		FD_SET(sockfd, &readSet);
//		//将所有客户端套接字加入读事件集合（读时间包括收到数据，新连接请求等）
//		for (const auto& client : clients) {
//			FD_SET(client.sockfd, &readSet);
//		}
//		//设置超时时间
//		timeval timeout;
//		timeout.tv_sec = 5;
//		timeout.tv_usec = 0;
//
//		//等待读事件发生
//		int activity = select(0, &readSet, NULL, NULL, &timeout);
//		if (activity == SOCKET_ERROR) {
//			std::cerr << "select error" << std::endl;
//			break;
//		}
//		else if (activity == 0) {
//			//超时，继续循环
//			continue;
//		}
//		//检查监听套接字是否有新连接（FD_ISSET检查是否某个套接字发生了什么读事件）
//		if (FD_ISSET(sockfd, &readSet)) {
//			sockaddr_in clientAddr;
//			int addrLen = sizeof(clientAddr);
//			SOCKET clientSock = accept(sockfd, (SOCKADDR*)&clientAddr, &addrLen);
//
//			if (clientSock == INVALID_SOCKET) {
//				std::cerr << "accept error" << std::endl;
//				continue;
//			}
//			u_long mode = 1; //设置非阻塞模式，防止因多个客户端连接导致程序卡住
//			if (ioctlsocket(clientSock, FIONBIO, &mode) == SOCKET_ERROR) {
//				std::cerr << "设置非阻塞模式失败，错误码：" << WSAGetLastError() << std::endl;
//				closesocket(clientSock);
//				continue;
//			}
//			//检查连接数是否超过最大限制
//			if (clients.size() < MAX) {
//				clients.emplace_back(clientSock);
//				std::cout << "New Client connected, current connection: " << clients.size() << std::endl;
//			}
//			else {
//				const char* msg = "Connection is limited，No Connection Accepted。\n";
//				send(clientSock, msg, strlen(msg), 0);
//				closesocket(clientSock);
//				std::cout << "Connection is limited，Connection denied。" << std::endl;
//			}
//		}
//		//接收客户端消息
//		for (size_t i = 0; i < clients.size();) {
//			SOCKET clientSock = clients[i].sockfd;
//			if (FD_ISSET(clientSock, &readSet)) {
//				char buffer[1024] = { 0 };
//				int recvLen = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
//				if (recvLen < 0) {
//					// 处理非阻塞模式下的错误
//					int err = WSAGetLastError();
//					if (err == WSAEWOULDBLOCK) {
//						// 正常情况：当前无数据可收，不关闭连接
//						i++;
//						continue;
//					}
//					else {
//						// 其他错误（如连接被强行关闭），视为断开
//						std::cout << "Client break (error: " << err << ")" << std::endl;
//						closesocket(clientSock);
//						clients.erase(clients.begin() + i);
//						BrodCastClientList(clients);
//					}
//				}
//				else if (recvLen == 0) {
//					// 正常断开（客户端主动关闭）
//					std::cout << "Client break (normal close)" << std::endl;
//					closesocket(clientSock);
//					clients.erase(clients.begin() + i);
//					BrodCastClientList(clients);
//				}
//				else {
//					buffer[recvLen] = '\0';
//					if (clients[i].name=="unname") {
//						// 设置用户名
//						clients[i].name = buffer;
//						std::cout << "Client set username: " << clients[i].name << std::endl;
//						//// 回复客户端确认
//						//const char* ok = "Connect success! you can start chating~\n";
//						//send(clientSock, ok, strlen(ok), 0);
//						BrodCastClientList(clients);
//					}
//					else {
//						// 转发消息给其他所有客户端（排除自己）
//						std::string msg = buffer;
//						
//						//若私聊
//						if (msg.substr(0, 1) == "@") {
//							size_t spacePos = msg.find(' ');	
//							if (spacePos == std::string::npos) {
//								// 格式错误
//								const char* errMsg = "私聊格式错误！正确格式: @用户名 消息内容\n";
//								send(clientSock, errMsg, strlen(errMsg), 0);
//								i++;
//								continue;
//							}
//							std::string targetName = msg.substr(1, spacePos - 1);
//							std::string content = msg.substr(spacePos + 1);
//							Client* targetClient = findClientByName(clients, targetName);
//							if (targetClient) {
//								std::string privateMsg = "[private_chat][" + clients[i].name + "]: " + content;
//								send(targetClient->sockfd, privateMsg.c_str(), privateMsg.size(), 0);
//								//send(clientSock, privateMsg.c_str(), privateMsg.size(), 0);
//								std::cout << "私聊消息: " << privateMsg << std::endl;
//							}
//							else {
//								const char* errMsg = "用户不存在，私聊失败！\n";
//								send(clientSock, errMsg, strlen(errMsg), 0);
//							}
//						}
//						else {
//							std::string publicMsg = "[" + clients[i].name + "]: " + msg;
//							std::cout << "公聊消息: " << publicMsg << std::endl;
//							for (const auto& c : clients) {
//								if (c.sockfd != clientSock) { // 不转发给发送者
//									send(c.sockfd, publicMsg.c_str(), publicMsg.size(), 0);
//								}
//							}
//						}
//						
//					}
//				}
//			}
//			else i++;	
//		}
//	}
//
//	closesocket(sockfd);
//	for (const auto& client : clients) {
//		closesocket(client.sockfd);
//	}
//	WSACleanup();
//
//	return 0;
//}