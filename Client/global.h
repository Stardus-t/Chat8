#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "Qstyle"
extern std::function<void(QWidget*)> repolish;
extern QString gate_url_prefix;
extern QString ChatServer_host;
extern QString ChatServer_port;

enum ReqId{//请求ID,为每个 HTTP 请求类型分配一个唯一的标识 ID，用于区分不同的请求业务。
    ID_GET_VARIFY_CODE=1001,//获取验证码
    ID_REG_USER=1002,//注册用户
    ID_LOGIN_USER=1003,
    ID_CHAT_LOGIN=1004,
    ID_GET_CHAT_SERVER_CONFIG=1005
};
enum ErrorCodes{//错误码枚举，定义可能出现的错误类型，用命名标识不同的错误原因。
    SUCCESS=0,
    ERR_JSON=1,//Json解析失败
    ERR_NETWORK=2,
};
enum Modules{//模块枚举，标识程序中的不同功能模块，用于区分请求所属的业务模块。
    REGISTERMOD=0,
    LOGINMOD=1
};

class GLOBAL
{
public:
    GLOBAL();
};

// enum TipErr{
//     TIP_SUCCESS = 0,
//     TIP_EMAIL_ERR = 1,
//     TIP_PWD_ERR = 2,
//     TIP_CONFIRM_ERR = 3,
//     TIP_PWD_CONFIRM = 4,
//     TIP_VARIFY_ERR = 5,
//     TIP_USER_ERR = 6
// };

struct ServerInfo{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

struct FriendInfo{
    int uid;
    QString name;
};
extern QVector<FriendInfo>friendList;


#endif // GLOBAL_H
