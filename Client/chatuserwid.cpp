#include "chatuserwid.h"
#include "ui_chatuserwid.h"
#include <QPixmap>

ChatUserWid::ChatUserWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ChatUserWid::SetInfo(QString name){
    _name=name;
    QPixmap pixmap(":/res/Login/head.png");
    ui->pic->setPixmap(pixmap.scaled(30,30,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->username->setText(name);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}
