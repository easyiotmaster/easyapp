#include "otherplatformssignin.h"
#include <QDesktopServices>
#include <QUrl>
OtherPlatformsSignIn::OtherPlatformsSignIn(QObject *parent) : QObject(parent)
{

    m_server = new QWebSocketServer("QWebChannel Standalone Example Server", QWebSocketServer::NonSecureMode);

    if (!m_server->listen(QHostAddress::LocalHost, 12345))
    {
        qFatal("Failed to open web socket server.");
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    m_clientWrapper = new WebSocketClientWrapper(m_server);

    // setup the channel
    bool check;
    check = QObject::connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected,&m_channel, &QWebChannel::connectTo);
    Q_ASSERT(check);

    m_channel.registerObject(QStringLiteral("dialog"), &m_socketHelper);

    check = connect(&m_socketHelper,SIGNAL(login(QString,QString)),SIGNAL(signIn(QString,QString)));
    Q_ASSERT(check);
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
    qDebug()<<url;
    QDesktopServices::openUrl(url);
}

void WebSocketServerHelper::receiveText(const QString &text)
{
    qDebug()<<"recvive message form websocket clietn:"<<text;
}

void WebSocketServerHelper::signIn(const QString &userName, const QString &password)
{
    emit login(userName,password);
}
