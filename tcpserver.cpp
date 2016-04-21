#include "tcpserver.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

TcpServer::TcpServer(QXmppClient* client,QObject *parent):QObject(parent),client(client)
{
    m_tcpServer = new QTcpServer(this);
    m_port = 5000;
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

void TcpServer::setTcpPort(quint16 port)
{
    stopAccept();

    m_port = port;
}

quint16 TcpServer::tcpPort()
{
    return m_port;
}

void TcpServer::recvSendMessage(const QJsonObject &jsonObject)
{
    if(jsonObject.contains("TO"))
    {
        QString jid = jsonObject.value("TO").toString("");
        QString body = jsonObject.value("BODY").toString("");
        if(jid == "")
        {
            qDebug()<<"Can not reslove To in SendMessage tcp packet";
            return;
        }
        emit sendMessage(jid,body);
        //client->sendMessage(jid,body);
    }
    else
        qDebug()<<"Can not find 'To'' in SendMessage tcp packet";
}

void TcpServer::recvSetPresence(const QJsonObject &jsonObject)
{
    if(jsonObject.contains("PRESENCE"))
    {
        QString status = jsonObject.value("PRESENCE").toString("");

        emit setPresenceStatus(status);
    }
    else
        qDebug()<<"Can not find 'PRESENCE' in SetPresence tcp packet";
}

void TcpServer::startAccept()
{
    m_tcpServer->listen(QHostAddress::Any,m_port);
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

    qDebug()<<data;

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
                    qDebug()<<"Json Analyze:Can not read TYPE";
                    break;
                case 0:
                    //SENDMESSAGE;
                    recvSendMessage(obj);
                    break;
                case 2:
                    //SETPRESENCE
                    recvSetPresence(obj);
                    break;
                default:
                    qDebug()<<"Json Analyze:Unrecognized Type";
                    break;
                }

            }
        }
    }
    else
        qDebug()<<jsonError.error;
}
