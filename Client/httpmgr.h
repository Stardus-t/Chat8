#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include "global.h"
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>

class HttpMgr:public QObject,public singleton<HttpMgr>,
                public std::enable_shared_from_this<HttpMgr>
//进行继承，使HttpMgr支持信号槽机制，单例模板类，自身创建指向自己的智能指针
{
    Q_OBJECT//这是 Qt 信号槽机制的标志，必须添加到使用信号或槽的类中
private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
public:
    void PostHttpReq(QUrl url,QJsonObject json,ReqId req_id,Modules mod);//发送http的post请求
    ~HttpMgr();
private:
    friend class singleton<HttpMgr>;// 允许单例模板类访问私有构造函数
    HttpMgr(); // 私有构造函数（单例模式核心，防止外部创建实例）
    QNetworkAccessManager _manager;// Qt 网络请求管理器（核心成员，用于发送 HTTP 请求）
signals:
    void sig_http_finish(ReqId id,QString res,ErrorCodes err,Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
