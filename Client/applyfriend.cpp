#include "applyfriend.h"
#include "ui_applyfriend.h"

ApplyFriend::ApplyFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplyFriend)
{
    ui->setupUi(this);
    connect(ui->ConfirmButton,&QPushButton::clicked,this,&ApplyFriend::addfirend);
    connect(ui->CancelButton,&QPushButton::clicked,this,&ApplyFriend::destroy_dlg);
}

void ApplyFriend::setTcpClient(TcpClient *client){
    _tcpClient=client;
}

void ApplyFriend::addfirend(){
    QString friend_name=ui->lineEdit->text().trimmed();
    if(friend_name.isEmpty()){
        ui->statelabel->setText("好友名不可为空");
        return ;
    }
    QString msg="ADDFRIEND|"+friend_name;
    _tcpClient->sendMessage(msg);
    qDebug() << "发送添加好友请求：" << msg;
    ui->statelabel->setText("好友申请发送成功");
}


ApplyFriend::~ApplyFriend()
{
    delete ui;
}
