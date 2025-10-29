#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
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

    ClickedLabel *add_friend=new ClickedLabel(this);
    QPixmap pixmap1(":/res/Login/addfirend.png");
    add_friend->setPixmap(pixmap1.scaled(30,30,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    add_friend->setGeometry(width()-562,10,30,30);
    connect(add_friend,&ClickedLabel::clicked,this,[this]{
        emit switchaddfriendPage();
    });

    ClickedLabel *search_his=new ClickedLabel(this);
    QPixmap pixmap2(":/res/Login/search.png");
    search_his->setPixmap(pixmap2.scaled(30,30,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    search_his->setGeometry(width()-200,10,30,30);
    connect(search_his,&ClickedLabel::clicked,this,[this]{
        emit search_his_page();
    });

    // 设置滚动区域
    ui->scrollArea->setWidget(chatContainer);//把空白控件塞进滚动区域
    ui->scrollArea->setWidgetResizable(true);

    for (const FriendInfo &friend_info : friendList) {
        QString friend_name = friend_info.name;
        ui->listWidget->addChatUser(friend_name);
        ui->listWidget->update();
        ui->listWidget->repaint();
    }

    tcpClient=new TcpClient(this);
    tcpClient->setUsername(username);

    connect(tcpClient, &TcpClient::sig_recvMessage, this, &ChatDialog::onRecvMessage);
    connect(tcpClient, &TcpClient::sig_error, this, &ChatDialog::onError);
    connect(this, &ChatDialog::onUserSelected, this, &ChatDialog::showWids);
    connect(ui->listWidget, &QListWidget::itemClicked, this, [this]() {
        emit onUserSelected();
    });

    QString chatHost ="127.0.0.1";
    quint16 chatPort = ChatServer_port.toUShort();
    qDebug()<<"ChatServer:"<<chatHost<<":"<<chatPort;
    tcpClient->connectToServer(chatHost, chatPort);
    ui->inputEdit->setHidden(true);
    ui->sendBtn->setHidden(true);
    ui->scrollArea->setHidden(true);


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
    m_bubbles.append(qMakePair(bubble, hLayout));
    ui->inputEdit->clear();

    // ui->chatRecord->append(QString("[%1] Me: %2")
    //                            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
    //                            .arg(send_msg));
    // ui->inputEdit->clear();
}


void ChatDialog::onRecvMessage(const QByteArray &msg)
{
    // qDebug() << "收到原始消息长度：" << msg.size();
    // qDebug() << "收到原始消息（十六进制）：" << msg.toHex();
    buffer_+=msg;
    while(true){
        if(exp_len==0){
            if(buffer_.size()<4){
                qDebug()<<"信息长度不足";
                break;
            }
            QByteArray len_bytes=buffer_.left(4);
            if(len_bytes.size()!=4){
                buffer_.clear();
                qDebug()<<"长度前缀格式错误";
                break;
            }
            uint32_t net_len;
            memcpy(&net_len, len_bytes.data(), 4);
            exp_len = qFromBigEndian<uint32_t>(net_len);
            buffer_.remove(0,4);
        }
        else{
            QByteArray content_bytes=buffer_;
            if(content_bytes.size()>=exp_len){
                QByteArray msg_bytes=content_bytes.left(exp_len);
                QString recv_msg=QString::fromUtf8(msg_bytes);
                buffer_.remove(0,exp_len);
                exp_len=0;
                // qDebug()<<"收到真实消息: "<<recv_msg;
                if(recv_msg.startsWith("USERLIST")){
                    analyUserList(recv_msg);
                    continue ;
                }
                else if(recv_msg.startsWith("Adding_friends_success")){
                    addonfriend(recv_msg);
                    continue ;
                }
                else if(recv_msg.startsWith("his")){
                    RecvHistory(recv_msg);
                    continue ;
                }
                else if(recv_msg.startsWith("SEARCH_RESULT")){
                    qDebug()<<"收到大模型回复";
                    emit get_LLA_res(recv_msg);
                    continue;
                }
                else{
                    int bracketPos1=recv_msg.indexOf(']');
                    QString sender_and_content=recv_msg.mid(bracketPos1+1);
                    int brackPos2=sender_and_content.indexOf(']');
                    QString content=sender_and_content.mid(brackPos2+1);


                    // QString content = recv_msg;

                    // 创建接收方气泡
                    BubbleFrame *bubble = new BubbleFrame(content, MessageType::RECEIVE, this);
                    QHBoxLayout *hLayout = new QHBoxLayout();
                    hLayout->setAlignment(Qt::AlignLeft); // 接收的消息靠左
                    hLayout->addWidget(bubble);
                    chatLayout->addLayout(hLayout);
                    m_bubbles.append(qMakePair(bubble, hLayout));

                    // 自动滚动到底部
                    QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
                    scrollBar->setValue(scrollBar->maximum());
                }

            }
            else{
                qDebug() << "期望长度:" << exp_len
                         << "实际内容字节数:" << content_bytes.size()
                         << "内容:" << content_bytes;
                qDebug()<<"数据长度小于期望长度";
                break;
            }
        }
    }
}

void ChatDialog::analyUserList(const QString &msg){
    QStringList parts=msg.split('|');
    if(parts.isEmpty()||parts.first()!="USERLIST"){
        qDebug()<<"用户信息表格式错误";
        return;
    }
    // ui->listWidget->clear();
    for(int i=1;i<parts.length();i++){
        QString username=parts[i];
        if(!username.isEmpty()){
            ui->listWidget->addChatUser(username);
            // QString other_user="wyf869003158@163.com";
            // ui->listWidget->addChatUser(other_user);
        }
    }
}

void ChatDialog::addonfriend(const QString &msg){
    QStringList parts=msg.split('|');
    if(parts.isEmpty()||parts.first()!="Adding_friends_success"){
        qDebug()<<"添加用户格式错误";
        return;
    }
    // ui->listWidget->clear();
    for(int i=1;i<parts.length();i++){
        QString username=parts[i];
        if(!username.isEmpty()){
            ui->listWidget->addChatUser(username);
            ui->listWidget->update();
            ui->listWidget->repaint();
        }
    }
}

void ChatDialog::RecvHistory(const QString &msg){
    int bracketPos=msg.indexOf(']');
    if(bracketPos==-1){
        qDebug()<<"消息格式错误，缺少‘]’";
        return ;
    }
    QString sender_and_content=msg.mid(bracketPos+1);
    int colonPos=sender_and_content.indexOf(':');
    if(colonPos==-1){
        qDebug()<<"消息格式错误，缺少‘:’";
        return ;
    }
    QString sender=sender_and_content.left(colonPos);
    QString content=sender_and_content.mid(colonPos+1);

    if(sender!=_username){
        BubbleFrame *bubble = new BubbleFrame(content, MessageType::RECEIVE, this);
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->setAlignment(Qt::AlignLeft);
        hLayout->addWidget(bubble);
        chatLayout->addLayout(hLayout);
        m_bubbles.append(qMakePair(bubble,hLayout));

        QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
    else{
        BubbleFrame *bubble = new BubbleFrame(content, MessageType::SEND, this);
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->setAlignment(Qt::AlignRight);
        hLayout->addWidget(bubble);
        chatLayout->addLayout(hLayout);
        m_bubbles.append(qMakePair(bubble,hLayout));

        QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}

void ChatDialog::onError(const QString &errMsg)
{
    QMessageBox::warning(this, "错误", errMsg);
}

void ChatDialog::clearChatHistory(){
    // 遍历所有气泡，逐个删除
    for (auto &pair : m_bubbles) {
        BubbleFrame *bubble = pair.first;
        QHBoxLayout *layout = pair.second;

        // 从父布局中移除子布局
        if (layout->parent() == chatLayout) {
            chatLayout->removeItem(layout);
        }

        // 删除气泡控件和布局
        delete bubble;
        delete layout;
    }
    // 清空列表
    m_bubbles.clear();
}

void ChatDialog::showWids(){
    clearChatHistory();
    ui->inputEdit->setHidden(false);
    ui->sendBtn->setHidden(false);
    ui->scrollArea->setHidden(false);
    //发送历史记录获取请求
    QString firendname;
    for(int i=0;i<ui->listWidget->count();i++){
        QListWidgetItem* item=ui->listWidget->item(i);
        if(item->isSelected()){
            ChatUserWid* userWid=qobject_cast<ChatUserWid*>(ui->listWidget->itemWidget(item));
            firendname=userWid->getName();
            break;
        }
    }
    QString msg="History|"+firendname;
    tcpClient->sendMessage(msg);
    qDebug()<<"获取历史信息请求发送成功:"<<msg;
}

ChatDialog::~ChatDialog()
{
    delete ui;
}
