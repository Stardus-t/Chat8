#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>
#include <winsock2.h>

class TcpClient :public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();
    void connectToServer(const QString &host,quint16 port);
    void sendMessage(const QString &msg);
    void setUsername(const QString &username);
signals:
    void sig_connected(bool success);
    void sig_recvMessage(const QByteArray &msg);
    void sig_error(const QString &errMsg);
private slots:
    void onConnection();
    void onDisconnection();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError err);
private:
    QTcpSocket *_socket;
    QString _username;
};

#endif // TCPCLIENT_H
