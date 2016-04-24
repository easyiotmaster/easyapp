#include "tcpserver.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "QXmppUtils.h"
TcpServer::TcpServer(QXmppClient* client, QObject *parent):QObject(parent),client(client)
{
    m_tcpServer = new QTcpServer(this);
    m_port = DEFAULT_SERVER_PORT;
    m_tcpServer->setMaxPendingConnections(MAX_SOCKET_LENGTH);
    connect(m_tcpServer,SIGNAL(newConnection()),this,SLOT(newConnect()));
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

TcpServer::JSON_ERROR TcpServer::recvSendMessage(const QJsonObject &jsonObject)
{
    JSON_ERROR errorCode = TcpServer::OK;
    if(jsonObject.contains("TO"))
    {
        QString bareJid = QXmppUtils::jidToBareJid(jsonObject.value("TO").toString(""));
        QString body = jsonObject.value("BODY").toString("");
        if(bareJid == "")
            errorCode = TcpServer::SM_JID_ERROR;
        else
            emit sendMessage(bareJid,body);
    }
    else
        errorCode = TcpServer::SM_NO_TO;
    return errorCode;
}

TcpServer::JSON_ERROR TcpServer::recvSetPresence(const QJsonObject &jsonObject)
{
    JSON_ERROR errorCode = TcpServer::OK;
    if(jsonObject.contains("PRESENCE"))
    {
        QString status = jsonObject.value("PRESENCE").toString("");

        emit setPresenceStatus(status);
    }
    else
        errorCode = TcpServer::SP_NO_PRESENCE;

    return errorCode;
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

void TcpServer::sendRecvMessage(const QXmppMessage& msg)
{
    if (msg.body().isEmpty())
        return;

    QString jid = msg.from();
    QString body = msg.body();

    QString ackStr = QObject::tr("{\"TYPE\":1,\"FROM\":\"%1\",\"BODY\":\"%2\"}").arg(jid).arg(body);

    for(int i = 0;i < m_socketList.size();++i)
        m_socketList[i]->write(ackStr.toLatin1(),ackStr.length());
}

void TcpServer::sendRecvPresence(const QString& jid, const QXmppPresence& pre)
{
    QString ackStr = QObject::tr("{\"TYPE\":3,\"FROM\":\"%1\",\"PRESENCE\":\"%2\"}").arg(jid).arg(pre.statusText());

    for(int i = 0;i < m_socketList.size();++i)
        m_socketList[i]->write(ackStr.toLatin1(),ackStr.length());
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

    JSON_ERROR errorCode = TcpServer::OK;

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data,&jsonError);
    if(jsonError.error == QJsonParseError::NoError)
    {
        if(jsonDoc.isObject())
        {
            QJsonObject obj = jsonDoc.object();
            if(obj.contains("TYPE"))
            {
                int type = obj.value("TYPE").toInt(-1);
                switch(type)
                {
                case -1:
                    break;
                case 0:
                    //SENDMESSAGE;
                    errorCode = recvSendMessage(obj);
                    break;
                case 2:
                    //SETPRESENCE
                    errorCode = recvSetPresence(obj);
                    break;
                default:
                    errorCode = TcpServer::TYPE_ERROR;
                    break;
                }
            }
            else
                errorCode = TcpServer::NO_TYPE;

        }
        else
            errorCode = TcpServer::NO_JSON_OBJECT;
    }
    else
        errorCode = TcpServer::NO_JSON;

    QString errorAckStr = QObject::tr("{\"ERROR\":%1}").arg(errorCode);

    tcpSocket->write(errorAckStr.toLatin1(),errorAckStr.length());
}
