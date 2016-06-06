#include "messagelog.h"
#include "QXmppUtils.h"
#include "utils.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
MessageLog::MessageLog(QObject *parent) : QObject(parent)
{

}

QString MessageLog::getMessageFileName(const QString &local_bareJid, const QString &remote_bareJid)
{
    QString msgPath = getSettingsDir(local_bareJid);
    QDir dir;
    if(!dir.exists(msgPath))
        dir.mkpath(msgPath);

    return msgPath + remote_bareJid + ".txt";
}

void MessageLog::append(const QString &local_bareJid, const QString &remote_bareJid, const QString &message)
{
    QString fileName = getMessageFileName(local_bareJid,remote_bareJid);
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Append))
        return;

    QTextStream stream(&file);
    stream<<message<<endl;
    file.close();
}

