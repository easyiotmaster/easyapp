#include "tcpserver.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "QXmppUtils.h"
TcpServer::TcpServer(QXmppClient* client, QObject *parent):QObject(parent),client(client)
{
    bool check;

    m_tcpServer = new QTcpServer(this);
    m_port = DEFAULT_SERVER_PORT;
    m_tcpServer->setMaxPendingConnections(MAX_SOCKET_LENGTH);
    m_translator = new QXmppTranslator(this);

    check = connect(m_tcpServer,SIGNAL(newConnection()),this,SLOT(newConnect()));
    Q_ASSERT(check);

    check = connect(m_translator,SIGNAL(sendMessage(QString,QString)),SIGNAL(sendMessage(QString,QString)));
    Q_ASSERT(check);

    check = connect(m_translator,SIGNAL(setPresenceStatus(QString)),SIGNAL(setPresenceStatus(QString)));
    Q_ASSERT(check);
}

TcpServer::~TcpServer()
{
    if(m_tcpServer->isListening())
        m_tcpServer->close();
    while(m_socketList.size())
    {
        m_socketList[0]->close();
    }
    m_socketList.clear();
    delete m_tcpServer;
}

quint16 TcpServer::tcpPort()
{
    return m_port;
}



void TcpServer::setTcpPort(quint16 port)
{
    stopAccept();

    m_port = port;
}

bool TcpServer::startAccept()
{
    return m_tcpServer->listen(QHostAddress::Any,m_port);
}

void TcpServer::stopAccept()
{
    if(m_tcpServer->isListening())
        m_tcpServer->close();

    while(m_socketList.size())
    {
        m_socketList[0]->close();
    }
    m_socketList.clear();
}

void TcpServer::sendRecvMessage(const QXmppMessage &msg)
{
    if (msg.body().isEmpty())
            return;
    QByteArray packet;
    m_translator->getRecvMessagePacket(msg,packet);

    for(int i = 0;i < m_socketList.size();++i)
        m_socketList[i]->write(packet);
}

void TcpServer::sendRecvPresence(const QString &jid, const QXmppPresence &pre)
{
    QByteArray packet;
    m_translator->getRecvPresencePacket(jid,pre,packet);

    for(int i = 0;i < m_socketList.size();++i)
        m_socketList[i]->write(packet);
}

void TcpServer::sendOnline(const QString &jid, bool online)
{
    QByteArray packet;
    m_translator->getOnlinePacket(jid,online,packet);

    for(int i = 0;i < m_socketList.size();++i)
        m_socketList[i]->write(packet);
}

void TcpServer::newConnect()
{
    QTcpSocket* tcpSocket = m_tcpServer->nextPendingConnection(); //得到每个连进来的socket

    if(tcpSocket == 0)
        return;

    m_socketList.append(tcpSocket);

    bool check;
    Q_UNUSED(check);

    check = connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readData())); //有可读的信息，触发读函数
    Q_ASSERT(check);

    check = connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnect()));
    Q_ASSERT(check);
}

void TcpServer::disConnect()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    m_socketList.removeOne(socket);

    bool check;
    Q_UNUSED(check);

    check = disconnect(socket,SIGNAL(readyRead()),this,SLOT(readData())); //
    Q_ASSERT(check);

    check = disconnect(socket,SIGNAL(disconnected()),this,SLOT(disConnect()));
    Q_ASSERT(check);

    socket->deleteLater();
}

void TcpServer::readData()
{
    QTcpSocket *tcpSocket = (QTcpSocket*)sender();
    QByteArray data = tcpSocket->readAll();

    QByteArray ackData;
    m_translator->recvData(data,ackData);

    tcpSocket->write(ackData);

}
