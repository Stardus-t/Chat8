#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "tcpclient.h"

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    void setTcpClient(TcpClient *client);
private slots:
    void addfirend();
signals:
    void destroy_dlg();
private:
    Ui::ApplyFriend *ui;
    TcpClient *_tcpClient;
};

#endif // APPLYFRIEND_H
