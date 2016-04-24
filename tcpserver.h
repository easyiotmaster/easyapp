#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "QTcpServer"
#include "QTcpSocket"
#include <QList>
#include <QJsonObject>
#include "QXmppClient.h"
#include "QXmppMessage.h"

#define MAX_SOCKET_LENGTH 5000
#define DEFAULT_SERVER_PORT 5000
class TcpServer : public QObject
{
    Q_OBJECT
private:

    QXmppClient*    client;
    QTcpServer* m_tcpServer;
    QList<QTcpSocket*> m_socketList;
    quint16     m_port;
public:
    enum JSON_ERROR
    {
        OK = 0,
        NO_JSON,
        NO_JSON_OBJECT,
        NO_TYPE,
        TYPE_ERROR,
        SM_NO_TO,
        SM_JID_ERROR,
        SP_NO_PRESENCE
    };

    explicit TcpServer(QXmppClient* client,QObject* parent = 0);
    ~TcpServer();


    quint16 tcpPort();

signals:
    void sendMessage(const QString &jid,const QString &body);
    void setPresenceStatus(const QString &status);
private:
    JSON_ERROR recvSendMessage(const QJsonObject &jsonObject);
    JSON_ERROR recvSetPresence(const QJsonObject &jsonObject);
public slots:
    void setTcpPort(quint16 port);
    bool startAccept();
    void stopAccept();

    void sendRecvMessage(const QXmppMessage& msg);//收到其他好友的消息，广播给给所有TCP客户端
    void sendRecvPresence(const QString& jid, const QXmppPresence& pre);//收到其他好友的状态，广播给所有TCP客户端

private slots:
    void newConnect();
    void disConnect();
    void readData();
};
#endif // TCPSERVER_H
