#include "chatuserlist.h"
#include <QListWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>
#include "chatuserwid.h"
#include <iostream>

ChatUserList::ChatUserList(QWidget *parent):QListWidget(parent) {
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);
    this->setResizeMode(QListWidget::Adjust);
    this->setUniformItemSizes(true);
    // connect(this,&QListWidget::itemClicked,this,[this](QListWidgetItem* item){
    //     ChatUserWid* userWid = qobject_cast<ChatUserWid*>(this->itemWidget(item));
    //     if(userWid) emit sig_userclicked(userWid->getName());
    // });
}

bool ChatUserList::eventFilter(QObject *watched,QEvent *event){
    //检查鼠标是否悬浮或离开
    if(watched==this->viewport()){
        if(event->type()==QEvent::Enter)
            //如果鼠标在里面，就显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        else if(event->type()==QEvent::Leave)
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff)            ;
    }

    //检查是否为鼠标滚轮事件
    if(watched==this->viewport()&&event->type()==QEvent::Wheel){
        //计算移动步数
        QWheelEvent *wheelEvent=dynamic_cast<QWheelEvent*>(event);
        int numDegrees=wheelEvent->angleDelta().y()/8;
        int numSteps=numDegrees/15;
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        //检查是否到底部
        QScrollBar *scrollbar=this->verticalScrollBar();
        int maxScrollValue=scrollbar->maximum();
        int currentScrollValue=scrollbar->value();
        if(maxScrollValue-currentScrollValue<=0){
            //滚动到底部，加载所有的联系人
            qDebug()<<"Load all chat user";
            emit sig_loadding_chat_user();
        };
        return true;
    }
    return QListWidget::eventFilter(watched,event);
}

void ChatUserList::addChatUser(QString username){
    QListWidgetItem *item=new QListWidgetItem(this);
    item->setSizeHint(QSize(0,60));
    ChatUserWid* userwid=new ChatUserWid(this);
    userwid->SetInfo(username);
    this->setItemWidget(item,userwid);
    std::cout<<"Adding username: "<<userwid->getName().toStdString()<<std::endl;
    qDebug()<<"设置列表控件成功";
}

void ChatUserList::resizeEvent(QResizeEvent *event) {
    QListWidget::resizeEvent(event);
    // 遍历所有项，更新尺寸
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        if (item) {
            // 宽度设为列表宽度（减去滚动条和边距），高度固定
            item->setSizeHint(QSize(width() - 20, 60));
        }
    }
}
