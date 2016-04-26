#ifndef QXMPPTRANSLATORDELEGATE_H
#define QXMPPTRANSLATORDELEGATE_H

#include <QObject>
#include <QXmppMessage.h>
#include <QXmppPresence.h>

class QXmppTranslatorDelegate : public QObject
{
    Q_OBJECT
public:
    explicit QXmppTranslatorDelegate(QObject *parent = 0);

signals:
    void sendMessage(const QString &jid, const QString &body);
    void setPresenceStatus(const QString &status);

public:
    virtual void getRecvMessagePacket(const QXmppMessage& msg, QByteArray &packet) = 0;
    virtual void getRecvPresencePacket(const QString &jid,const QXmppPresence &pre, QByteArray &packet) = 0;
    virtual void getOnlinePacket(const QString &jid,bool online,QByteArray &packet) = 0;
    virtual void recvData(const QByteArray &recvData, QByteArray &ackData) = 0;
};

#endif // QXMPPTRANSLATORDELEGATE_H
