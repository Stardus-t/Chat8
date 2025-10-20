#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QRegularExpression>
#include "global.h"
#include "httpmgr.h"
#include <QTimer>

QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;

RegisterDialog::RegisterDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterDialog)
    , _countdown(5)
{
    ui->setupUi(this);
    initHttpHandlers();
    ui->passwordlineEdit->setEchoMode(QLineEdit::Password);
    ui->conernpasswordlineEdit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state","normal");
    connect(ui->v_codepushButton,&QPushButton::clicked,this,&RegisterDialog::on_v_codepushButton_clicked);
    connect(ui->cancelpushButton,&QPushButton::clicked,this,&RegisterDialog::exitRegister);
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_reg_mod_finish,this,&RegisterDialog::slot_reg_mod_finish);
    _countdown_timer = new QTimer(this);
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
    });
}


void RegisterDialog::on_sure_btn_clicked(){
    if(ui->usernamelineEdit->text()==""){
        showTip(tr("用户名不可为空"),false);
        return;
    }
    if(ui->passwordlineEdit->text()==""){
        showTip(tr("密码不可为空"),false);
        return;
    }
    if(ui->conernpasswordlineEdit->text()==""){
        showTip(tr("请确认密码"),false);
    }
    if(ui->emaillineEdit->text()==""){
        showTip(tr("邮箱不可为空"),false);
        return;
    }
    if(ui->v_codelineEdit->text()==""){
        showTip(tr("验证码不可为空"),false);
        return;
    }
    QJsonObject objson;
    objson["user"]=ui->usernamelineEdit->text();
    objson["password"]=ui->passwordlineEdit->text();
    objson["confirm"]=ui->conernpasswordlineEdit->text();
    objson["email"]=ui->emaillineEdit->text();
    objson["varifycode"]=ui->v_codelineEdit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),objson,ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

void RegisterDialog::on_v_codepushButton_clicked(){
    auto email=ui->emaillineEdit->text();
    QRegularExpression regex("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.\\w+)+");
    bool match=regex.match(email).hasMatch();
    if(match){
        QJsonObject json_obj;
        json_obj["email"]=email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),json_obj,
            ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);
        qDebug()<<gate_url_prefix+"/get_varifycode";
    }
    else{
        showTip(tr("邮箱地址不正确"),false);
    }
}

void RegisterDialog::showTip(QString str,bool b_ok){
    if(!b_ok)
        ui->err_tip->setProperty("state","err");
    else
        ui->err_tip->setProperty("state","normal");
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err){
    if(err!=ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }
    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc=QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }
    QJsonObject jsonObj=jsonDoc.object();
    _handlers[id](jsonObj);
    return;
}

void RegisterDialog::initHttpHandlers(){
    //注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,[this](QJsonObject jsonObj){
        int error=jsonObj["error"].toInt();//获取错误码
        qDebug()<<error;
        if(error!=ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email=jsonObj["email"].toString();
        showTip(tr("验证码已发送到邮箱"),true);
        qDebug()<<"email is"<<email;
    });
    //注册回包逻辑
    _handlers.insert(ReqId::ID_REG_USER,[this](QJsonObject jsonObj){
        int error=jsonObj["error"].toInt();//获取错误码
        qDebug()<<error;
        if(error!=ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email=jsonObj["email"].toString();
        showTip(tr("用户注册成功"),true);
        qDebug()<<"email is"<<email;
        qDebug()<< "user uuid is " <<  jsonObj["uuid"].toString();
        ChangeTipPage();
    });
}

void RegisterDialog::ChangeTipPage()
{
    _countdown = 5;
    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
    showTip(tr("注册成功，5秒后返回登录界面..."), true);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}
