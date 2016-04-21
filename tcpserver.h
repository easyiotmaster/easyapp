#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "QTcpServer"
#include "QTcpSocket"
#include <QList>
#include <QJsonObject>
#include "QXmppClient.h"
#define MAX_SOCKET_LENGTH 5000

class TcpServer : public QObject
{
    Q_OBJECT
private:
    QXmppClient*    client;
    QTcpServer* m_tcpServer;
    QList<QTcpSocket*> m_socketList;
    quint16     m_port;
public:
    explicit TcpServer(QXmppClient* client,QObject* parent = 0);
    ~TcpServer();

    void setTcpPort(quint16 port);
    quint16 tcpPort();
signals:
    void sendMessage(const QString &jid,const QString &body);
    void setPresenceStatus(const QString &status);
private:
    void recvSendMessage(const QJsonObject &jsonObject);
    void recvSetPresence(const QJsonObject &jsonObject);
public slots:
    void startAccept();
    void stopAccept();

private slots:
    void newConnect();
    void disConnect();
    void readData();
};

#endif // TCPSERVER_H
