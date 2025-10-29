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
#include "bubbleframe.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDialog(const QString &username,QWidget *parent = nullptr);
    ~ChatDialog();
    TcpClient* getTcpClient() { return tcpClient; }
private slots:
    void on_sendBtn_clicked();
    void onRecvMessage(const QByteArray &msg);
    void onError(const QString &errMsg);
    void showWids();
    void RecvHistory(const QString &msg);
signals:
    void switchProfiePage();
    void onUserSelected();
    void switchaddfriendPage();
    void search_his_page();
    void get_LLA_res(QString& res);
private:
    Ui::ChatDialog *ui;
    TcpClient *tcpClient;
    QString _username;
    QByteArray buffer_;
    uint32_t exp_len=0;
    void analyUserList(const QString &msg);
    void addonfriend(const QString &msg);
    void clearChatHistory();
    // void sendHistory_req();
    QWidget *chatContainer;
    QVBoxLayout *chatLayout;
    QList<QPair<BubbleFrame*, QHBoxLayout*>> m_bubbles;
};

class ClickedLabel:public QLabel
{
    Q_OBJECT
public:
    explicit ClickedLabel(QWidget *parent=nullptr):QLabel(parent){
        setCursor(Qt::PointingHandCursor);
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
