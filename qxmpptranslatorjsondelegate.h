#ifndef QXMPPTRANSLATORJSONDELEGATE_H
#define QXMPPTRANSLATORJSONDELEGATE_H

#include <QObject>
#include "QJsonObject"
#include "qxmpptranslatordelegate.h"
class QXmppTranslatorJsonDelegate : public QXmppTranslatorDelegate
{
    Q_OBJECT
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

    explicit QXmppTranslatorJsonDelegate(QObject *parent = 0);

    void getRecvMessagePacket(const QXmppMessage& msg, QByteArray &packet);
    void getRecvPresencePacket(const QString &jid,const QXmppPresence &pre, QByteArray &packet);
    void getOnlinePacket(const QString &jid,bool online,QByteArray &packet);
    void recvData(const QByteArray &recvData, QByteArray &ackData);

private:
    JSON_ERROR recvSendMessage(const QJsonObject &jsonObject);
    JSON_ERROR recvSetPresence(const QJsonObject &jsonObject);

};

#endif // QXMPPTRANSLATORJSONDELEGATE_H
