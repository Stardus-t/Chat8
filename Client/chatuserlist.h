#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H
#include <QListWidget>
#include <QString>

class ChatUserList:public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent=nullptr);
    void addChatUser(QString Username);

protected:
    bool eventFilter(QObject *watched,QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void sig_loadding_chat_user();
    // void sig_userclicked(const QString friendname);
};

#endif // CHATUSERLIST_H
