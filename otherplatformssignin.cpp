#include "otherplatformssignin.h"
#include <QDesktopServices>
#include <QUrl>
OtherPlatformsSignIn::OtherPlatformsSignIn(QObject *parent) : QObject(parent)
{

    m_server = new QWebSocketServer("QWebChannel Standalone Example Server", QWebSocketServer::NonSecureMode);

    m_port = 12345;
    while(!m_server->listen(QHostAddress::LocalHost, m_port))
    {
        m_port = 1000+qrand()%60000;
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
        //url = QUrl(QObject::tr("http://www.easy-iot.cc/qqconnect/example//index.html?webChannelBaseUrl=ws://127.0.0.1:%1").arg(m_port));
        url = QUrl(QObject::tr("http://www.easy-iot.cc/qqconnect/example/oauth/index.php?port=%1").arg(m_port));
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
