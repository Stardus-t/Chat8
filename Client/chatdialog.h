#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "tcpclient.h"
#include <QString>
#include <QWidget>
#include <QVBoxLayout>
#include <Qlabel>
#include <QMouseEvent>
#include <QObject>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDialog(const QString &username,QWidget *parent = nullptr);
    ~ChatDialog();
private slots:
    void on_sendBtn_clicked();
    void onRecvMessage(const QString &msg);
    void onError(const QString &errMsg);
signals:
    void switchProfiePage();

private:
    Ui::ChatDialog *ui;
    TcpClient *tcpClient;
    QString _username;
    void analyUserList(const QString &msg);
    QWidget *chatContainer;
    QVBoxLayout *chatLayout;
};

class ClickedLabel:public QLabel
{
    Q_OBJECT
public:
    explicit ClickedLabel(QWidget *parent=nullptr):QLabel(parent){
        setCursor(Qt::PointingHandCursor);//设置鼠标悬停时设置手形光标
    }
signals:
    void clicked();
protected:
    void mouseReleaseEvent(QMouseEvent *event)override{
        if(event->button()==Qt::LeftButton) emit clicked();
        QLabel::mouseReleaseEvent(event);
    }
};

#endif // CHATDIALOG_H
