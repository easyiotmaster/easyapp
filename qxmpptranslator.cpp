#include "qxmpptranslator.h"
#include <QDebug>
QXmppTranslator::QXmppTranslator(QObject *parent) : QObject(parent)
{
    bool check;
    m_delegate = new QXmppTranslatorJsonDelegate(this);

    check = connect(m_delegate,SIGNAL(sendMessage(QString,QString)),SIGNAL(sendMessage(QString,QString)));
    Q_ASSERT(check);

    check = connect(m_delegate,SIGNAL(setPresenceStatus(QString)),SIGNAL(setPresenceStatus(QString)));
    Q_ASSERT(check);
}

QXmppTranslator::~QXmppTranslator()
{
    delete m_delegate;
}

void QXmppTranslator::setTranslatorDelegate(QXmppTranslatorDelegate *delegate)
{
    bool check;

    check = m_delegate->disconnect();
    Q_ASSERT(check);

    delete m_delegate;
    m_delegate = delegate;

    check = connect(m_delegate,SIGNAL(sendMessage(QString,QString)),SIGNAL(sendMessage(QString,QString)));
    Q_ASSERT(check);

    check = connect(m_delegate,SIGNAL(setPresenceStatus(QString)),SIGNAL(setPresenceStatus(QString)));
    Q_ASSERT(check);
}

void QXmppTranslator::getRecvMessagePacket(const QXmppMessage &msg, QByteArray &packet)
{
    m_delegate->getRecvMessagePacket(msg,packet);
}

void QXmppTranslator::getRecvPresencePacket(const QString &jid, const QXmppPresence &pre, QByteArray &packet)
{
    m_delegate->getRecvPresencePacket(jid,pre,packet);
}

void QXmppTranslator::getOnlinePacket(const QString &jid, bool online, QByteArray &packet)
{
    m_delegate->getOnlinePacket(jid,online,packet);
}

void QXmppTranslator::recvData(const QByteArray &recvData, QByteArray &ackData)
{
    m_delegate->recvData(recvData,ackData);
}
