#ifndef OTASETDIALOG_H
#define OTASETDIALOG_H

#include <QDialog>
#include <QXmppClient.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QHttpMultiPart>
#include <QTimer>

#define HTTPTIMEROUT    30000
#define ATTIMEOUT       30000
namespace Ui {
class OTASetDialog;
}

class OTASetDialog : public QDialog
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

    Ui::OTASetDialog*   ui;
    QXmppClient*        m_client;
    QString             m_barejid;
    QStringList         m_resourceList;
    QString             m_currentResource;
    QNetworkAccessManager* m_manager;
    QString             m_fileName;
    QString             m_fileMD5;
    QFile*              m_file;
    QHttpMultiPart*     m_multiPart;
    QTimer*             m_downTimer;
    DOWNLOAD_ACTION     m_lastDownloadAction;

public:
    explicit OTASetDialog(QWidget *parent = 0);
    ~OTASetDialog();

    void setQXmppClient(QXmppClient* client);
    void setBareJid(const QString &bareJid);
    void setResource(const QStringList &resouces);
private:
    void lockUI(bool lock);
    bool isResourceExist(const QString &resource);
private slots:
    void selectFWFile();
    void remoteDownload();
    void currentSourceChange(int index);
    void replyFinish(QNetworkReply* reply);
    void replyFinished();
    void replyError(QNetworkReply::NetworkError);
    void uploadProgress(qint64 sendSize,qint64 totalSize);
    void downloadAckTimeout();
public slots:
    int recvClientMessage(const QString &msg);
};

#endif // OTASETDIALOG_H
