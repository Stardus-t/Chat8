#include "httpmgr.h"
#include <QNetworkReply>

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod){
    //创建一个HTTP POST请求，并设置请求头和请求体
    QByteArray data=QJsonDocument(json).toJson();//这里的json是具体的数据
    //通过Url构造请求，服务器关于本次请求的关键元信息（如数据格式、数据长度等）
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));
    //发送请求，处理响应，获取自己的智能指针，构造伪闭包并增加智能指针引用计数
    auto self=shared_from_this();
    QNetworkReply *reply=_manager.post(request,data);//发送请求
    //设置信号和槽等待发送完成
    QObject::connect(reply,&QNetworkReply::finished,[reply,self,req_id,mod](){
        if(reply->error()!=QNetworkReply::NoError){
            qDebug()<<reply->errorString();
            //发送信号通知完成
            emit self->sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod);//发送信号通知失败
            reply->deleteLater();
            return;
        }
        //无错误则读回请求
        QString res=reply->readAll();
        //发送信号通知完成
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod){
    if(mod==Modules::REGISTERMOD){
        //发送信号通知注册模块http响应结束
        emit sig_reg_mod_finish(id,res,err);
    }
    if(mod == Modules::LOGINMOD){
        emit sig_login_mod_finish(id, res, err);
    }
}

HttpMgr::HttpMgr() {
    connect(this,&HttpMgr::sig_http_finish,this,&HttpMgr::slot_http_finish);
}

HttpMgr::~HttpMgr(){

}
