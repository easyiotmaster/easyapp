#include "qxmpptranslatorjsondelegate.h"
#include <QJsonDocument>
#include <QXmppUtils.h>

QXmppTranslatorJsonDelegate::QXmppTranslatorJsonDelegate(QObject *parent):QXmppTranslatorDelegate(parent)
{

}

void QXmppTranslatorJsonDelegate::getRecvMessagePacket(const QXmppMessage &msg, QByteArray &packet)
{
    QString jid = msg.from();
    QString body = msg.body();

    QString ackStr = QObject::tr("{\"TYPE\":1,\"FROM\":\"%1\",\"BODY\":\"%2\"}").arg(jid).arg(body);

    packet = ackStr.toLatin1();
}

void QXmppTranslatorJsonDelegate::getRecvPresencePacket(const QString &jid, const QXmppPresence &pre, QByteArray &packet)
{
    QString ackStr = QObject::tr("{\"TYPE\":3,\"FROM\":\"%1\",\"PRESENCE\":\"%2\"}").arg(jid).arg(pre.statusText());
    packet = ackStr.toLatin1();
}

void QXmppTranslatorJsonDelegate::getOnlinePacket(const QString &jid, bool online, QByteArray &packet)
{
    QString ackStr;
    if(online)
        ackStr = QObject::tr("{\"TYPE\":4,\"JID\":\"%1\"}").arg(jid);
    else
        ackStr = QObject::tr("{\"TYPE\":5,\"JID\":\"%1\"}").arg(jid);

    packet = ackStr.toLatin1();
}

void QXmppTranslatorJsonDelegate::recvData(const QByteArray &recvData, QByteArray &ackData)
{
    JSON_ERROR errorCode = QXmppTranslatorJsonDelegate::OK;

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(recvData,&jsonError);
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
                    errorCode = QXmppTranslatorJsonDelegate::TYPE_ERROR;
                    break;
                }
            }
            else
                errorCode = QXmppTranslatorJsonDelegate::NO_TYPE;

        }
        else
            errorCode = QXmppTranslatorJsonDelegate::NO_JSON_OBJECT;
    }
    else
        errorCode = QXmppTranslatorJsonDelegate::NO_JSON;

    QString errorAckStr = QObject::tr("{\"ERROR\":%1}").arg(errorCode);
    ackData = errorAckStr.toLatin1();
}

QXmppTranslatorJsonDelegate::JSON_ERROR QXmppTranslatorJsonDelegate::recvSendMessage(const QJsonObject &jsonObject)
{
    JSON_ERROR errorCode = QXmppTranslatorJsonDelegate::OK;
    if(jsonObject.contains("TO"))
    {
        QString bareJid = QXmppUtils::jidToBareJid(jsonObject.value("TO").toString(""));
        QString body = jsonObject.value("BODY").toString("");
        if(bareJid == "")
            errorCode = QXmppTranslatorJsonDelegate::SM_JID_ERROR;
        else
            emit sendMessage(bareJid,body);
    }
    else
        errorCode = QXmppTranslatorJsonDelegate::SM_NO_TO;
    return errorCode;
}

QXmppTranslatorJsonDelegate::JSON_ERROR QXmppTranslatorJsonDelegate::recvSetPresence(const QJsonObject &jsonObject)
{
    JSON_ERROR errorCode = QXmppTranslatorJsonDelegate::OK;
    if(jsonObject.contains("PRESENCE"))
    {
        QString status = jsonObject.value("PRESENCE").toString("");

        emit setPresenceStatus(status);
    }
    else
        errorCode = QXmppTranslatorJsonDelegate::SP_NO_PRESENCE;

    return errorCode;
}


