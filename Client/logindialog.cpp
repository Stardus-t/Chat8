#include "logindialog.h"
#include "ui_logindialog.h"
#include "global.h"
#include "httpmgr.h"
#include <QMessageBox>
#include <QJsonArray>

LoginDialog::LoginDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->RegisterBtn,&QPushButton::clicked,this,&LoginDialog::switchRegister);
    initHttpHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);
}

bool LoginDialog::checkUserValid(){

    auto user = ui->userlineEdit->text();
    if(user.isEmpty()){
        qDebug() << "User empty " ;
        return false;
    }

    return true;
}

bool LoginDialog::checkPwdValid(){
    auto pwd = ui->passwordlineEdit->text();
    if(pwd.length() < 6 || pwd.length() > 15){
        qDebug() << "Pass length invalid";
        return false;
    }

    return true;
}

void LoginDialog::on_LoginBtn_clicked(){
    qDebug()<<"login btn clicked";
    if(checkUserValid()==false){
        ui->error_msg->setStyleSheet("color: red;");
        ui->error_msg->setText("用户不存在");

        return ;
    }
    if(checkPwdValid()==false){
        ui->error_msg->setText("密码错误");
        ui->error_msg->setStyleSheet("color: red;");
        return ;
    }
    auto user=ui->userlineEdit->text();
    auto pwd=ui->passwordlineEdit->text();
    QJsonObject json_obj;
    json_obj["user"]=user;
    json_obj["password"]=pwd;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}

void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            qDebug()<<"参数错误";
            return;
        }
        auto user = jsonObj["user"].toString();
        ChatServer_host= jsonObj["host"].toString();
        ChatServer_port= jsonObj["port"].toString();
        // qDebug()<<"ChatServer:"<<ChatServer_host<<":"<<ChatServer_port;

        qDebug()<< "user is " << user <<"登录成功";
        QJsonArray friendsArray=jsonObj["friends"].toArray();
        for(const QJsonValue& value:friendsArray){
            QJsonObject friendObj=value.toObject();
            FriendInfo info;
            info.uid=friendObj["uid"].toInt();
            info.name=friendObj["name"].toString();
            qDebug()<<"好友用户名为："<<info.name;
            friendList.append(info);
        }
        emit switchChat();
    });
}


void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
         qDebug()<<"网络请求错误";
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
         qDebug()<<"json解析错误";
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
         qDebug()<<"json解析错误";
        return;
    }

    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}

bool LoginDialog::enableBtn(bool enabled)
{
    ui->LoginBtn->setEnabled(enabled);
    ui->RegisterBtn->setEnabled(enabled);
    return true;
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{

    if(bsuccess){
        qDebug()<<"聊天服务连接成功，正在登录...";
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

    }else{
        qDebug()<<"网络异常";
        enableBtn(true);
    }

}

LoginDialog::~LoginDialog()
{
    delete ui;
}
