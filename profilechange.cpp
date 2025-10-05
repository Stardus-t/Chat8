#include "profilechange.h"
#include "ui_profilechange.h"

ProfileChange::ProfileChange(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileChange)
{
    ui->setupUi(this);
}

ProfileChange::~ProfileChange()
{
    delete ui;
}
