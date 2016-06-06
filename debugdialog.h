#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>
#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include <QHttpMultiPart>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#define HTTPTIMEROUT    30000
#define ATTIMEOUT       30000

namespace Ui {
class DebugDialog;
}

class DebugDialog : public QDialog
{
    Q_OBJECT
private:
    enum DOWNLOAD_ACTION
    {
        UPLOAD_TO_HTTPSERVER = 0,
        AT_DOWN,
        AT_ISP,
        AT_RST
    };

    Ui::DebugDialog* ui;
    QXmppClient*    m_client;// holds a reference to the the connected client
    QString         m_bareJid;
    QString         m_displayName;
    QFile           *m_downloadFile;
    QString         m_fileMD5;
    QHttpMultiPart*     m_multiPart;
    QNetworkAccessManager* m_manager;
    QTimer              *m_downTimer;
    DOWNLOAD_ACTION     m_lastDownloadAction;

    QString stringToHtmlFilter(const QString &str);
    QString stringToHtml(const QString &str,QColor color);
    QString getDownloadFileName();
    QString getDownloadResource();
    int     parseRemoteDownloadACK(const QString &msg);
    void    insertRecvMessageToTextBrowser(const QString &msg);
    void    insertSendMessageToTextBrowser(const QString &msg);
    void    insertPresenceToTextBrowser(const QString &msg);
    void    insertParseTextToTextBrowser(const QString &msg);
public:
    explicit DebugDialog(QWidget *parent = 0);
    ~DebugDialog();

    void show();
    QString getBareJid() const;
    QString getDisplayName() const;
    void setBareJid(const QString &bareJid);
    void setDisplayName(const QString &displayName);
    void setResource(const QStringList &resouces);
    void setQXmppClient(QXmppClient* client);
    void messageReceived(const QXmppMessage &msg);
    void presenceReceived(const QXmppPresence &presence);

public slots:
    void sendMessage();
    void remoteDownload();
    void replyError(QNetworkReply::NetworkError error);
    void uploadProgress(qint64 sendSize, qint64 totalSize);
    void uploadFinish();
    void downloadTimeout();
protected:
    void keyPressEvent(QKeyEvent* event);
};

#endif // DEBUGDIALOG_H
