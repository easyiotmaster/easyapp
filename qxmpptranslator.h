#ifndef QXMPPTRANSLATOR_H
#define QXMPPTRANSLATOR_H

#include <QObject>
#include <QXmppMessage.h>
#include <QXmppPresence.h>
#include "qxmpptranslatordelegate.h"
#include "qxmpptranslatorjsondelegate.h"

class QXmppTranslator : public QObject
{
    Q_OBJECT
private:
    QXmppTranslatorDelegate* m_delegate;
public:
    explicit QXmppTranslator(QObject *parent = 0);
    ~QXmppTranslator();

    void setTranslatorDelegate(QXmppTranslatorDelegate *delegate);

signals:
    void sendMessage(const QString &jid,const QString &body);
    void setPresenceStatus(const QString &status);

public slots:
    void getRecvMessagePacket(const QXmppMessage &msg, QByteArray &packet);//收到其他好友的消息，广播给给所有TCP客户端
    void getRecvPresencePacket(const QString &jid, const QXmppPresence &pre, QByteArray &packet);//收到其他好友的状态，广播给所有TCP客户端
    void getOnlinePacket(const QString &jid,bool online,QByteArray &packet);
    void recvData(const QByteArray &recvData, QByteArray &ackData);
};

#endif // QXMPPTRANSLATOR_H
