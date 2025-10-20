#include "profilechange.h"
#include "ui_profilechange.h"

ProfileChange::ProfileChange(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileChange)
{
    ui->setupUi(this);
    connect(ui->Cancel_btn,&QPushButton::clicked,this,&ProfileChange::destroyPage);
    QPixmap pixmap;
    pixmap.load(":/res/Login/head.png");
    ui->label->setPixmap(pixmap);
}

ProfileChange::~ProfileChange()
{
    delete ui;
}
