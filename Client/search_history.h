#ifndef SEARCH_HISTORY_H
#define SEARCH_HISTORY_H

#include <QDialog>
#include "tcpclient.h"
#include "chatdialog.h"

namespace Ui {
class Search_history;
}

class Search_history : public QDialog
{
    Q_OBJECT

public:
    explicit Search_history(QWidget *parent = nullptr);
    ~Search_history();
    void setTcpClient(TcpClient *client);
    void sendMsg();
    void add_response(const QString& res);
    void setChatDialog(ChatDialog *dialog);
signals:
    void destroy_dlg();
private:
    Ui::Search_history *ui;
    TcpClient *_tcpClient;
    ChatDialog *_chatDialog;
};

#endif // SEARCH_HISTORY_H
