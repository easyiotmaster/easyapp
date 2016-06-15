#ifndef OHTERPLATFORMSSIGNIN_H
#define OHTERPLATFORMSSIGNIN_H

#include <QObject>
#include <QtWebSockets/QWebSocketServer>
#include "webchannel/websocketclientwrapper.h"
#include "webchannel/websockettransport.h"
#include "QWebChannel"

class WebSocketServerHelper :public QObject
{
    Q_OBJECT
signals:
    void sendText(const QString &text);
    void login(const QString &userName,const QString &password);
public slots:
    void receiveText(const QString &text);
    void signIn(const QString &userName,const QString &password);
};


class OtherPlatformsSignIn : public QObject
{
    Q_OBJECT
private:
    WebSocketServerHelper   m_socketHelper;
    QWebSocketServer*       m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebChannel             m_channel;
    uint                    m_port;
public:
    enum SIGNIN_PLATFORM
    {
        WECHAT,
        QQ
    };
    explicit OtherPlatformsSignIn(QObject *parent = 0);
    ~OtherPlatformsSignIn();
signals:
    void signIn(const QString &userName,const QString &password );
public slots:
    void signIn(SIGNIN_PLATFORM platform);
};


#endif // OHTERPLATFORMSSIGNIN_H
