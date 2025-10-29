#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent):QObject(parent) {
    _socket=new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &TcpClient::onConnection);
    connect(_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnection);
    connect(_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(_socket, &QTcpSocket::errorOccurred, this, &TcpClient::onErrorOccurred);
}

TcpClient::~TcpClient(){
    if(_socket->isOpen())  _socket->disconnectFromHost();
}

void TcpClient::connectToServer(const QString &host, quint16 port){
    _socket->connectToHost(QHostAddress(host),port);
}

void TcpClient::sendMessage(const QString &msg){
    if(_socket->state()!=QTcpSocket::ConnectedState){
        emit sig_error("未连接到服务器");
        return;
    }
    QByteArray data=msg.toUtf8();
    _socket->write(data);
}

void TcpClient::setUsername(const QString &username){
    _username=username;
}

void TcpClient::onConnection(){
    qDebug() << "TCP连接成功";
    emit sig_connected(true);
    if (!_username.isEmpty()) {
        sendMessage(_username);
    }
}

void TcpClient::onDisconnection()
{
    qDebug() << "TCP断开连接";
    emit sig_connected(false);
}

void TcpClient::onReadyRead(){
    QByteArray data=_socket->readAll();
    // qDebug() << "客户端收到原始二进制：" << data.toHex();
    emit sig_recvMessage(data);
    // QString msg=QString ::fromUtf8(data);
    // emit sig_recvMessage(msg);
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError err)
{
    QString errMsg = _socket->errorString();
    qDebug() << "TCP错误：" << errMsg;
    emit sig_error(errMsg);
}
