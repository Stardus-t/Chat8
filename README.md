# Chat8
## 核心组件与功能
### 客户端(client)
采用单例模式设计（singleton.h），确保线程安全的实例管理
实现聊天气泡 UI 组件（bubbleframe.cpp），支持文本显示与重绘
### 聊天服务器(ChatServer/Chatserver2)
提供异步消息发送与接收功能（ClientSession.h / ClientSession.cpp）
基于 gRPC 实现服务间通信（chat_service.grpc.pb.cc / .h）
使用 Protocol Buffers 进行消息序列化（chat_service.pb.cc）
支持多服务器部署，代码结构基本一致
### 网关服务器(GateServer)
基于 Node.js 开发，提供验证码发送功能（server.js）
使用 Redis 存储验证码并设置过期时间
依赖多个 npm 包（如uuid、lodash等），见package-lock.json
### 状态服务器(StatusServer)
提供 UUID 生成功能（StatusServiceImpl.cpp）
基于 gRPC 提供服务发现相关接口（message.grpc.pb.h）
### 辅助功能
聊天历史记录处理脚本（History_msg_dealer.py），使用阿里云 DashScope API 进行消息分析
聊天记录存储示例（temped.txt）
## 技术细节
### 网络通信
客户端 - 服务器采用 TCP 协议，消息格式含 4 字节长度前缀
服务间通信使用 gRPC，定义了TextChatMsg等核心接口
异步 I/O 操作基于 asio 库实现
### 数据处理
消息序列化采用 Protocol Buffers
Redis 用于存储临时数据（如验证码）
聊天历史记录以文本文件形式存储
### 设计模式
客户端使用单例模式管理全局实例
服务器端采用会话模式（ClientSession）管理客户端连接
### 安全验证
实现邮件验证码功能，有效期 10 分钟
验证码通过 UUID 生成，截取前 4 位使用
## 依赖与配置
客户端依赖：Qt 框架、C++11 及以上
服务器依赖：gRPC、Protobuf、asio、Boost（UUID 生成）
网关依赖：Node.js、Redis 客户端、邮件发送模块
第三方服务：阿里云 DashScope API（用于消息分析）
## 消息流程
客户端通过网关服务器获取验证码完成注册 / 登录
状态服务器维护用户与聊天服务器的映射关系
聊天服务器处理实际消息的发送与接收，支持多服务器集群
聊天历史记录可通过 Python 脚本进行分析处理
