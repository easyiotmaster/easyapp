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
#include "at/atparse.h"
#include <QProcess>
#include <QFileDialog>
#define HTTP_TIMEROUT    30000
#define AT_TIMEOUT       30000
#define TASK_TIMEOUT     30000
namespace Ui {
class debugWindow;
}

class DebugDialog : public QMainWindow
{
    Q_OBJECT
private:
    enum AT_CMD_TYPE{
        AT_NULL,
        AT_DOWN,
        AT_ISP,
        AT_REST,
        AT_GETPIC,
        AT_OTA,
        AT_MSG,
        AT_HEX,
        AT_SQLRESP,
        AT_QUERY_SIGNAL
    };

    enum TASK
    {
        TASK_NULL,
        TASK_REMOTE_UPDATE,
        TASK_LED_UPDATE,
        TASK_OTA_UPDATE,
        TASK_GET_PIC,
        TASK_SHOW_PIC
    };

    enum BROWSER_TEXT_TYPE//在文本框中显示的消息类型
    {
        TEXT_UNKNOW,
        TEXT_RECV_MESSAGE,//接收到的消息
        TEXT_SEND_MESSAGE,//发送的消息
        TEXT_PRESENCE_TEXT,//出席信息
        TEXT_REMOTE_UPDATE,
        TEXT_LED_UPDATE,//LED更新相关
        TEXT_GETPIC,
        TEXT_OTA_UPDATE,
        TEXT_SHOWPIC,
        TEXT_SQLQUERY,
        TEXT_HEX,
        TEXT_SENDHEX,
        TEXT_AT_CMD,
        TEXT_SIG,
        TEXT_ONLINE
    };

    enum DOWN_DEV_TYPE
    {
        STM32F10X,
        AVR
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
    QTimer*         m_taskTimer;
    QTimer*         m_atTimer;
    QTimer*         m_waitTurnFileCloseTimer;
    QTimer*         m_querySigTimer;
    QMenu*          m_toolMenu;
    QAction*        m_remoteDownloadAction;
    QAction*        m_updateLCDAction;
    QAction*        m_getPicAction;
    QAction*        m_otaAction;
    int             m_downloadFileLen;
    int             m_downloadDev;
    TASK            m_currentTask;
    QString         m_currentAtCmdStr;
    AT_CMD_TYPE     m_currentAtCmdType;
    QProcess*       m_turnHexProcess;
    QString         m_turnFileName;
    QMap<int,bool>  m_sqlPnResultMap;
    bool            m_online;

    QFileDialog*    m_selectFileDialog;
    QString stringToHtmlFilter(const QString &str);//生成HTML文本
    QString stringToHtml(const QString &str,QColor color);//生成带颜色的HTML文本
    QString imagePathToHtml(const QString &path);
    QString getDownloadFileName();//选择一个远程下载文件
    QString getLedFileName();//选择一个LED更新文件
    QString getDownloadResource();//要下载的resource类型

    bool    isWaitingAtCmdAck();//等待AT指令的发送成功回复
    void    sendAtCmd(const QString &atStr,AT_CMD_TYPE at_type,int sendType = 0);    //发送At指令
    void    parseAtCmdString(const QString &str);   //解析AT指令
    void    parseAtDownCmd(ATCMD_DATA &ad);     //解析AT DOWN指令
    void    parseAtISPCmd(ATCMD_DATA &ad);      //解析AT ISP指令
    void    parseAtGETPICCmd(ATCMD_DATA &ad);   //解析AT GETPIC指令
    void    parseAtPICCmd(ATCMD_DATA &ad);      //解析AT PIC指令
    void    parseAtSQLCmd(ATCMD_DATA &ad);      //解析AT SQL指令
    void    parseAtFWCmd(ATCMD_DATA &ad);       //解析AT FW指令
    void    parseAtHWCmd(ATCMD_DATA &ad);       //解析AT HW指令
    void    parseAtERRCmd(ATCMD_DATA &ad);      // at 错误指令
    void    parseAtOKCmd();     //解析AT OK指令
    void    parseAtHEXCmd(ATCMD_DATA &ad);      //解析AT HEX指令
    void    parseAtSIGCmd(ATCMD_DATA &ad);      //解析AT SIG指令
    void    parseAtINFOCmd(ATCMD_DATA &ad);      //解析AT INFO指令
    void    insertTextToTextBrowser(const QString &msg,BROWSER_TEXT_TYPE type);//插入消息到文本框
    void    insertImgToBrowser(const QString &str); //收到图片类型的BASE64数据
    void    createToolMenu();           //创建工具菜单
    void    httpUpload(const QString &fileName);//远程下载文件
    void    geLEDTemplateFirmware();    //下载LED模板文件
    void    exitCurrentTask();          //退出当前任务
    BROWSER_TEXT_TYPE getBrowserTextType(TASK task);
    void    turnHex2Bin(const QString &fileName);
    QString    turnFileType2Bin(const QString &filePathName);

public:
    explicit DebugDialog(QWidget *parent = 0);
    ~DebugDialog();

    void show();
    QString getBareJid() const;
    QString getDisplayName() const;
    void setBareJid(const QString &bareJid);
    void setDisplayName(const QString &displayName);
    void setResource(const QStringList &resources);
    void setQXmppClient(QXmppClient* client);
    void messageReceived(const QXmppMessage &msg);
    void presenceReceived(const QXmppPresence &presence);
    void setBareJidOnline(bool online);
signals:
    void updateSignal(const QString & bareJid,int signal , int version);
public slots:
    void sendMessage();//点击发送按钮
    void sendHexMessage();//发送AT+HEX指令
    void sendSqlResp(int pn ,bool ok);
    void remoteUpdate();//点击远程下载
    void updateLEDFirmware();//点击更新LED
    void getPic();//点击获取图片
    void otaUpdate();

    void httpUploadProgress(qint64 sendSize, qint64 totalSize); //更新上传文件进度
    void httpUploadFinish();    //上传文件完成
    void httpUploadReplayError(QNetworkReply::NetworkError error);  //远程下载上传文件HTTP访问出错

    void httpDownloadProgress(qint64 recvSize,qint64 totalSize);//更新下载LED文件进度

    void httpDownloadFinish();  //下载LED文件完成
    void httpDownloadError(QNetworkReply::NetworkError error);      //下载LED文件错误

    void imageDownloadFinish(); //下载图片完成
    void imageDownloadError(QNetworkReply::NetworkError error);     //下载图片错误

    void taskTimeout();
    void atCmdTimeout();
    void readHttpData();        //保存LED文件

    void turnHexProcessError(QProcess::ProcessError);//转换程序执行失败
    void turnHexFinished(int); //转换程序执行完成
    void waitTurnFileCloseTimeout();

    void querySignal();

    void fileSelected(const QString &fileName);
protected:
    void keyPressEvent(QKeyEvent* event);
};

#endif // DEBUGDIALOG_H
