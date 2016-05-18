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

public slots:
    void receiveText(const QString &text);
};


class OtherPlatformsSignIn : public QObject
{
    Q_OBJECT
private:
    WebSocketServerHelper   m_socketHelper;
    QWebSocketServer*       m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebChannel             m_channel;
public:
    enum SIGNIN_PLATFORM
    {
        WECHAT
    };
    explicit OtherPlatformsSignIn(QObject *parent = 0);
    ~OtherPlatformsSignIn();
signals:

public slots:
    void signIn(SIGNIN_PLATFORM platform);
};


#endif // OHTERPLATFORMSSIGNIN_H
