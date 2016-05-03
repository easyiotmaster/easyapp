#ifndef OTASETDIALOG_H
#define OTASETDIALOG_H

#include <QDialog>
#include <QXmppClient.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
namespace Ui {
class OTASetDialog;
}

class OTASetDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::OTASetDialog*   ui;
    QXmppClient*        m_client;
    QString             m_barejid;
    QStringList         m_resourceList;
    QString             m_currentResource;
    QNetworkAccessManager* manager;
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
    void replyError(QNetworkReply::NetworkError);
    void uploadProgress(qint64 sendSize,qint64 totalSize);
};

#endif // OTASETDIALOG_H
