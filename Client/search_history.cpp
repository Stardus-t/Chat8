#include "search_history.h"
#include "ui_search_history.h"

Search_history::Search_history(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Search_history)
{
    ui->setupUi(this);
    connect(ui->cancelButton,&QPushButton::clicked,this,&Search_history::destroy_dlg);
    connect(ui->sendButton,&QPushButton::clicked,this,&Search_history::sendMsg);
}

void Search_history::setTcpClient(TcpClient *client){
    _tcpClient=client;
}

void Search_history::sendMsg(){
    QString friend_name=ui->nametextedit->toPlainText().trimmed();
    QString msg=ui->msgtextedit->toPlainText().trimmed();
    if(friend_name.isEmpty()||msg.isEmpty()){
        qDebug()<<"搜索历史消息为空";
        return ;
    }
    QString send_msg="SEARCH_HISTORY|"+friend_name+"|"+msg;
    _tcpClient->sendMessage(send_msg);
    qDebug() << "发送搜索历史消息：" << send_msg;
}

void Search_history::add_response(const QString& res){
    ui->textBrowser->append(res);
}

void Search_history::setChatDialog(ChatDialog *dialog) {
    _chatDialog = dialog;
    if (_chatDialog) {
        connect(_chatDialog, &ChatDialog::get_LLA_res,this, &Search_history::add_response);
    }
}

Search_history::~Search_history()
{
    delete ui;
}
