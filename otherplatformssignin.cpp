#include "otherplatformssignin.h"
#include <QDesktopServices>
#include <QUrl>
OtherPlatformsSignIn::OtherPlatformsSignIn(QObject *parent) : QObject(parent)
{
    m_server = new QWebSocketServer("QWebChannel Standalone Example Server", QWebSocketServer::NonSecureMode);
    //m_server.setServerName(QStringLiteral("QWebChannel Standalone Example Server"));
    //m_server.setSslConfiguration(QWebSocketServer::NonSecureMode);

    if (!m_server->listen(QHostAddress::LocalHost, 12345))
    {
        qFatal("Failed to open web socket server.");
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    m_clientWrapper = new WebSocketClientWrapper(m_server);

    // setup the channel
    QObject::connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected,
                     &m_channel, &QWebChannel::connectTo);

    m_channel.registerObject(QStringLiteral("dialog"), &m_socketHelper);
}

OtherPlatformsSignIn::~OtherPlatformsSignIn()
{
    m_server->close();
    m_clientWrapper->deleteLater();
}

void OtherPlatformsSignIn::signIn(OtherPlatformsSignIn::SIGNIN_PLATFORM platform)
{
    // open a browser window with the client HTML page
    QUrl url;
    switch(platform)
    {
    case WECHAT:
        url = QUrl("http://115.28.44.147/hqq/index.html?webChannelBaseUrl=ws://127.0.0.1:12345");
        break;
    default:
        break;
    }

    QDesktopServices::openUrl(url);
}

void WebSocketServerHelper::receiveText(const QString &text)
{
    qDebug()<<"recvive message form websocket clietn:"<<text;
}
