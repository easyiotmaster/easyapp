#ifndef MESSAGELOG_H
#define MESSAGELOG_H

#include <QObject>

class MessageLog : public QObject
{
    Q_OBJECT
public:
    explicit MessageLog(QObject *parent = 0);    
public:
    static QString getMessageFileName(const QString &local_bareJid,const QString &remote_bareJid);
    static void append(const QString &local_bareJid,const QString &remote_bareJid,const QString &message);
};

#endif // MESSAGELOG_H
