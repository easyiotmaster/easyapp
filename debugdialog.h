#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H
#include <QMainWindow>
#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"
#include <QHttpMultiPart>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QMenu>
#include <QAction>

#define HTTPTIMEROUT    30000
#define ATTIMEOUT       30000

namespace Ui {
class debugWindow;
}

class DebugDialog : public QMainWindow
{
    Q_OBJECT
private:
    enum DOWNLOAD_ACTION//远程下载状态
    {
        UPLOAD_TO_HTTPSERVER = 0,//正在上传到HTTP服务器
        AT_DOWN,//正在DOWN
        AT_ISP,//正在ATISP
        AT_RST//正在RST
    };
    enum BROWSER_TEXT_TYPE//在文本框中显示的消息类型
    {
        RECV_MESSAGE,//接收到的消息
        SEND_MESSAGE,//发送的消息
        PRESENCE_TEXT,//出席信息
        REMOTEDOWNLOAD,//远程下载相关
        LEDUPDATE//LED更新相关
    };

    Ui::debugWindow* ui;
    QXmppClient*    m_client;// holds a reference to the the connected client
    QString         m_bareJid;
    QString         m_displayName;
    QFile           *m_downloadFile;
    QFile           m_ledFirmwareFile;
    QFile           m_ledTemplateFirmwareFile;
    QString         m_fileMD5;
    QHttpMultiPart* m_multiPart;
    QNetworkAccessManager* m_manager;
    QTimer*         m_remoteDownloadTimer;
    QTimer*         m_httpDownloadTimer;
    DOWNLOAD_ACTION m_lastDownloadAction;
    QMenu*          m_toolMenu;
    QAction*        m_remoteDownloadAction;
    QAction*        m_updateLCDAction;

    QString stringToHtmlFilter(const QString &str);//生成HTML文本
    QString stringToHtml(const QString &str,QColor color);//生成带颜色的HTML文本
    QString getDownloadFileName();//选择一个远程下载文件
    QString getLedFileName();//选择一个LED更新文件
    QString getDownloadResource();//要下载的resource类型
    int     parseRemoteDownloadACK(const QString &msg);//解析远程下载的回复信息
    void    insertTextToTextBrowser(const QString &msg,BROWSER_TEXT_TYPE type);//插入消息到文本框
    void    createToolMenu();//创建工具菜单
    void    remoteDownload(const QString &fileName);//远程下载
    void    geLEDTemplateFirmware();//下载LED模板文件
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
    void remoteDownloadReplayError(QNetworkReply::NetworkError error);
    void uploadProgress(qint64 sendSize, qint64 totalSize);
    void downloadProgress(qint64 recvSize,qint64 totalSize);
    void uploadFinish();
    void remoteDownloadTimeout();
    void httpDownloadTimeout();
    void updateLEDFirmware();
    void readHttpData();
    void httpDownloadFinish();
protected:
    void keyPressEvent(QKeyEvent* event);
};

#endif // DEBUGDIALOG_H
