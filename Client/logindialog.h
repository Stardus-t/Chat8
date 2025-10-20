#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ui_logindialog.h"
#include "global.h"
#include <QWidget>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QWidget
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    bool checkUserValid();
    bool checkPwdValid();
    void initHttpHandlers();
    bool enableBtn(bool enabled);
    QString getUsername() const { return ui->userlineEdit->text(); }

signals:
    void switchRegister();
    void sig_connect_tcp(ServerInfo);
    void switchChat();


private:
    Ui::LoginDialog *ui;
    QMap<ReqId, std::function<void(QJsonObject)>> _handlers;
    int _uid;
    QString _token;

private slots:
    void on_LoginBtn_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_con_finish(bool bsuccess);
};

#endif // LOGINDIALOG_H
