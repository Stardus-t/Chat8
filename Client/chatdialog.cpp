#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
#include "bubbleframe.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPixmap>
#include "chatuserwid.h"
#include "global.h"

ChatDialog::ChatDialog(const QString &username,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatDialog)
    , _username(username)
{
    ui->setupUi(this);

    // 初始化聊天容器
    chatContainer = new QWidget();//空白控件
    chatLayout = new QVBoxLayout(chatContainer);//垂直布局器，消息逐渐往下排
    chatLayout->setAlignment(Qt::AlignTop);
    chatLayout->setSpacing(10);

    //可点击标签
    ClickedLabel *imageLabel=new ClickedLabel(this);
    QPixmap pixmap(":/res/Login/head.png");
    imageLabel->setPixmap(pixmap.scaled(30,30,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    imageLabel->setGeometry(width()-40,10,30,30);

    connect(imageLabel,&ClickedLabel::clicked,this,[this]{
        emit switchProfiePage();
    });

    // 设置滚动区域
    ui->scrollArea->setWidget(chatContainer);//把空白控件塞进滚动区域
    ui->scrollArea->setWidgetResizable(true);

    tcpClient=new TcpClient(this);
    tcpClient->setUsername(username);

    connect(tcpClient, &TcpClient::sig_recvMessage, this, &ChatDialog::onRecvMessage);
    connect(tcpClient, &TcpClient::sig_error, this, &ChatDialog::onError);


    QString chatHost ="127.0.0.1";
    quint16 chatPort = ChatServer_port.toUShort();
    qDebug()<<"ChatServer:"<<chatHost<<":"<<chatPort;
    tcpClient->connectToServer(chatHost, chatPort);

}

void ChatDialog::on_sendBtn_clicked(){
    QString msg=ui->inputEdit->toPlainText().trimmed();
    if(msg.isEmpty()) return;
    QString username;
    for(int i=0;i<ui->listWidget->count();i++){
        QListWidgetItem* item=ui->listWidget->item(i);
        if(item->isSelected()){
            ChatUserWid* userWid=qobject_cast<ChatUserWid*>(ui->listWidget->itemWidget(item));
            username=userWid->getName();
            break;
        }
    }
    qDebug()<<"选择的用户名"<<username;
    QString send_msg="@"+username+" "+msg;
    qDebug()<<"发送的消息"<<send_msg;


    tcpClient->sendMessage(send_msg);

    BubbleFrame *bubble = new BubbleFrame(msg, MessageType::SEND, this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setAlignment(Qt::AlignRight); // 发送的消息靠右
    hLayout->addWidget(bubble);
    chatLayout->addLayout(hLayout);
    ui->inputEdit->clear();

    // ui->chatRecord->append(QString("[%1] Me: %2")
    //                            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
    //                            .arg(send_msg));
    // ui->inputEdit->clear();
}

void ChatDialog::onRecvMessage(const QString &msg)
{
    if(msg.startsWith("USERLIST")){
        analyUserList(msg);
        return ;
    }

    QString content = msg; // 这里需要根据实际消息格式解析

    // 创建接收方气泡
    BubbleFrame *bubble = new BubbleFrame(content, MessageType::RECEIVE, this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setAlignment(Qt::AlignLeft); // 接收的消息靠左
    hLayout->addWidget(bubble);
    chatLayout->addLayout(hLayout);

    // 自动滚动到底部
    QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());

    // ui->chatRecord->append(QString("[%1] The other user: %2")
    //                            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
    //                            .arg(msg));
}

void ChatDialog::analyUserList(const QString &msg){
    QStringList parts=msg.split('|');
    if(parts.isEmpty()||parts.first()!="USERLIST"){
        qDebug()<<"用户信息表格式错误";
        return;
    }
    ui->listWidget->clear();
    for(int i=1;i<parts.length();i++){
        QString username=parts[i];
        if(!username.isEmpty()){
            ui->listWidget->addChatUser(username);
            QString other_user="wyf869003158@163.com";
            ui->listWidget->addChatUser(other_user);
        }
    }
}

void ChatDialog::onError(const QString &errMsg)
{
    QMessageBox::warning(this, "错误", errMsg);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}
