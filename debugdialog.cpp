#include "debugdialog.h"
#include "ui_debugwindow.h"
#include <QTime>
#include "messagelog.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFileDialog>
#include "iniconfig.h"
#include <QCryptographicHash>
#include <QHttpPart>
#include <QNetworkReply>
#include <QKeyEvent>
#include <QInputDialog>
#include "at/conv_hex_str.h"
#include "sqlhelper.h"

QString DebugDialog::stringToHtmlFilter(const QString &str)
{
    QString htmlStr = str;
    htmlStr.replace("&","&amp;");
    htmlStr.replace(">","&gt;");
    htmlStr.replace("<","&lt;");
    htmlStr.replace("\"","&quot;");
    htmlStr.replace("\'","&#39;");
    htmlStr.replace(" ","&nbsp;");
    htmlStr.replace("\n","<br>");
    htmlStr.replace("\r","<br>");

    return htmlStr;
}

QString DebugDialog::stringToHtml(const QString &str, QColor color)
{
    QString htmlStr = stringToHtmlFilter(str);
    QByteArray array;
    array.append(color.red());
    array.append(color.green());
    array.append(color.blue());
    QString strC(array.toHex());
    return QString("<span style=\" color:#%1;\">%2</span><br>").arg(strC).arg(htmlStr);
}

QString DebugDialog::imagePathToHtml(const QString &path)
{
    return QString("<img src=\"%1\"/><br>").arg(path);
}

QString DebugDialog::getDownloadFileName()
{
    QString fileName;
    if(m_downloadDev == STM32F10X)
    {
        fileName = QFileDialog::getOpenFileName(this,tr("STM32固件"),IniConfig::getRemoteDownloadPath(m_client->configuration().jidBare()),"BIN(*.bin)");
        //m_selectDownFileDialog->setFilter("BIN(*.bin)");
       // m_selectDownFileDialog->open();
    }
    else
    {
        fileName = QFileDialog::getOpenFileName(this,tr("AVR固件"),IniConfig::getRemoteDownloadPath(m_client->configuration().jidBare()),"HEX(*.hex)");
        //m_selectDownFileDialog->setFilter("BIN(*.bin)");
    }
   // m_selectDownFileDialog->open();
    if(!fileName.isEmpty())
        IniConfig::setRemoteDownloadPath(fileName,m_client->configuration().jidBare());
    return fileName;
}

QString DebugDialog::getLedFileName()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("LED固件"),IniConfig::getLedFilePath(m_client->configuration().jidBare()),"BIN(*.*)");
    IniConfig::setLedFilePath(fileName,m_client->configuration().jidBare());
    return fileName;
}

QString DebugDialog::getDownloadResource()
{
    //return ui->cmb_resource->currentText();
    return "";
}

bool DebugDialog::isWaitingAtCmdAck()
{
    if(m_atTimer->isActive())
        return true;
    else
        return false;
}

void DebugDialog::sendAtCmd(const QString &atStr, AT_CMD_TYPE at_type, int sendType)
{
    if(sendType == 1)
    {
        m_client->sendMessage(m_bareJid,atStr);
        insertTextToTextBrowser(atStr,TEXT_SEND_MESSAGE);
    }
    else
    {
        if(isWaitingAtCmdAck())
        {
            insertTextToTextBrowser(QString("cannot send:is waiting for last atcmd ack,last:%1,new:%2").arg(m_currentAtCmdStr).arg(atStr),TEXT_AT_CMD);
        }
        else
        {
            m_client->sendMessage(m_bareJid,atStr);
            m_currentAtCmdType = at_type;
            m_currentAtCmdStr = atStr;
            insertTextToTextBrowser(atStr,TEXT_SEND_MESSAGE);
            m_atTimer->start(AT_TIMEOUT);
        }
    }
}

void DebugDialog::parseAtCmdString(const QString &str)
{
    ATCMD_DATA at;

    QByteArray ba  = str.toLatin1();
    int isAt = parse_at_cmd_string(ba.data(),&at);//判断是否为AT指令
    if(isAt == 0)
    {
        QString atName = QString(at.name);
        qDebug()<<"ATCMD:"<<atName<<str.length()<<str.size();
        if(atName == "DOWN")
        {
            parseAtDownCmd(at);
        }
        else if(atName == "ISP")
        {
            parseAtISPCmd(at);
        }
        else if(atName == "GETPIC")
        {
            parseAtGETPICCmd(at);
        }
        else if(atName == "PIC")
        {
            parseAtPICCmd(at);
        }
        else if(atName == "SQL")
        {
            parseAtSQLCmd(at);
        }
        else if(atName == "FW")
        {
            parseAtFWCmd(at);
        }
        else if(atName == "HW")
        {
            parseAtHWCmd(at);
        }
        else if(atName == "ERR")
        {
            parseAtERRCmd(at);
        }
        else if(atName == "HEX")
        {
            parseAtHEXCmd(at);
        }
        else if(atName == "SIG")
        {
            parseAtSIGCmd(at);
        }
        else if(atName == "INFO")
        {
            parseAtINFOCmd(at);
        }
    }
    else if(str == "OK")
    {
        parseAtOKCmd();
    }
}

void DebugDialog::parseAtDownCmd(ATCMD_DATA &ad)
{
    if(m_currentTask == TASK_NULL)
        return;
    qDebug()<<__func__<<ad.param_count;
    int down_type = -1;
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        down_type = get_at_param_int(&ad,0);
        if(down_type != 2)
            return;
    }
    else if(ad.param_count == 2)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        down_type = get_at_param_int(&ad,0);
        if(down_type != 1)
            return;

        int type1 = ad.param[1].type;
        if(type1 != ATCMD_PARAM_STRING_TYPE)
            return;
    }
    else if(ad.param_count == 3)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        down_type = get_at_param_int(&ad,0);
        if(down_type != 0)
            return;

        int type1 = ad.param[1].type;
        int type2 = ad.param[2].type;
        if(type1 != ATCMD_PARAM_INTEGER_TYPE || type2 != ATCMD_PARAM_INTEGER_TYPE)
            return;
    }
    else
        return;

    qDebug()<<"down_cmd_type:"<<down_type;
    switch(down_type)
    {
    case 0://下载进度
    {
        int clen = get_at_param_int(&ad,1);
        int tlen = get_at_param_int(&ad,2);
        if(tlen != 0)
        {
            int percentage = clen*100/tlen;
            ui->pgb_download->show();
            ui->pgb_download->setValue(percentage);
            m_taskTimer->start(TASK_TIMEOUT);
        }
        break;
    }
    case 1://下载完成
    {
        char outbuff[32*1024];
        QString md5 = QString(get_at_param_string(&ad,1,outbuff,sizeof(outbuff)));
        if(md5 != m_fileMD5)
        {
            insertTextToTextBrowser("MD5 does not match",getBrowserTextType(m_currentTask));
            exitCurrentTask();
        }
        else
        {
            if(m_currentTask == TASK_REMOTE_UPDATE || m_currentTask ==  TASK_LED_UPDATE)
            {
                if(m_downloadFileLen)
                {

                    QString atString = tr("AT+ISP=%1,%2").arg(m_downloadFileLen).arg(m_downloadDev);
                    sendAtCmd(atString,AT_ISP);
                    ui->pgb_download->setValue(0);
                    m_taskTimer->start(TASK_TIMEOUT);
                }
                else
                {
                    insertTextToTextBrowser("download file error,can not isp",getBrowserTextType(m_currentTask));
                    exitCurrentTask();
                }
            }
            else if(m_currentTask == TASK_OTA_UPDATE)
            {
                QString atString = QString("AT+OTA=\"%1\"").arg(md5);
                sendAtCmd(atString,AT_OTA);
                m_taskTimer->start(TASK_TIMEOUT);
            }
        }
        break;
    }
    case 2://下载错误
    {
        insertTextToTextBrowser("download failed",getBrowserTextType(m_currentTask));
        exitCurrentTask();
        break;
    }
    default:
        break;
    }
}

void DebugDialog::parseAtISPCmd(ATCMD_DATA &ad)
{
    if(m_currentTask == TASK_NULL)
        return;

    int isp_type = -1;
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        isp_type = get_at_param_int(&ad,0);
        if(isp_type != 1 && isp_type != 2)
            return;
    }
    else if(ad.param_count == 3)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        isp_type = get_at_param_int(&ad,0);
        if(isp_type != 0)
            return;

        int type1 = ad.param[1].type;
        int type2 = ad.param[2].type;
        if(type1 != ATCMD_PARAM_INTEGER_TYPE || type2 != ATCMD_PARAM_INTEGER_TYPE)
            return;
    }
    else
        return;

    switch(isp_type)
    {
    case 0:
    {
        int clen = get_at_param_int(&ad,1);
        int tlen = get_at_param_int(&ad,2);
        if(tlen != 0)
        {
            int percentage = clen*100/tlen;
            ui->pgb_download->show();
            ui->pgb_download->setValue(percentage);
            //m_remoteDownloadTimer->start(AT_TIMEOUT);
            m_taskTimer->start(TASK_TIMEOUT);
        }
        break;
    }
    case 1:
    {
        ui->pgb_download->setValue(100);
        insertTextToTextBrowser("remote download successed",getBrowserTextType(m_currentTask));
        exitCurrentTask();
        break;
    }
    case 2:
    {
        insertTextToTextBrowser("isp failed",getBrowserTextType(m_currentTask));
        exitCurrentTask();
        break;
    }
    default:
        break;
    }
}

void DebugDialog::parseAtGETPICCmd(ATCMD_DATA &ad)
{
    if(m_currentTask == TASK_NULL)
        return;

    int cmdType = -1;
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        cmdType = get_at_param_int(&ad,0);
        if(cmdType != 1 && cmdType != 2)
            return;
    }
    else if(ad.param_count == 3)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        cmdType = get_at_param_int(&ad,0);
        if(cmdType != 0)
            return;

        int type1 = ad.param[1].type;
        int type2 = ad.param[2].type;
        if(type1 != ATCMD_PARAM_INTEGER_TYPE || type2 != ATCMD_PARAM_INTEGER_TYPE)
            return;
    }
    else
        return;

    switch(cmdType)
    {
    case 0:
    {
        int clen = get_at_param_int(&ad,1);
        int tlen = get_at_param_int(&ad,2);
        if(tlen != 0)
        {
            int percentage = clen*100/tlen;
            ui->pgb_download->show();
            ui->pgb_download->setValue(percentage);
            //m_getPicTimer->start(AT_TIMEOUT);
            m_taskTimer->start(TASK_TIMEOUT);
        }
        break;
    }
    case 1:
        ui->pgb_download->setValue(100);
        insertTextToTextBrowser("picture upload finished",getBrowserTextType(m_currentTask));
        exitCurrentTask();
        break;
    case 2:
        insertTextToTextBrowser("picture upload failed",getBrowserTextType(m_currentTask));
        exitCurrentTask();
        break;
    default:
        break;
    }
}

void DebugDialog::parseAtPICCmd(ATCMD_DATA &ad)
{
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_STRING_TYPE)
            return;
    }
    else
        return;
    if(m_currentTask == TASK_NULL || m_currentTask == TASK_GET_PIC)
    {
        ui->pgb_download->hide();
        m_taskTimer->start(TASK_TIMEOUT);
        m_currentTask = TASK_SHOW_PIC;

        char out_buff[32*1024];
        QString url = QString(get_at_param_string(&ad,0,out_buff,sizeof(32)));

        insertTextToTextBrowser(QString("start downloading:%1").arg(url),getBrowserTextType(m_currentTask));
        QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(url)));
        connect(reply,SIGNAL(finished()),SLOT(imageDownloadFinish()));
        connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),SLOT(imageDownloadError(QNetworkReply::NetworkError)));
    }
    else
    {
        insertTextToTextBrowser("is performing other task",TEXT_REMOTE_UPDATE);
    }
}

void DebugDialog::parseAtSQLCmd(ATCMD_DATA &ad)
{
    if(ad.param_count == 2)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;
        int type1 = ad.param[1].type;
        if(type1 != ATCMD_PARAM_STRING_TYPE)
            return;
    }
    else
        return;

    char out_buff[32*1024];
    int pn = get_at_param_int(&ad,0);
    /*if(m_sqlPnResultMap.contains(pn))
    {
        sendSqlResp(pn,m_sqlPnResultMap[pn]);
    }
    else
    {
        if(m_sqlPnResultMap.size()>=32)
            m_sqlPnResultMap.remove(m_sqlPnResultMap.firstKey());
        bool ok;
        QString sql = QString(get_at_param_string(&ad,1,out_buff,sizeof(out_buff)));
        insertTextToTextBrowser("start query mysql database",TEXT_SQLQUERY);
        insertTextToTextBrowser(SqlHelper::queryMySqlDatabase(sql,ok),TEXT_SQLQUERY);
        m_sqlPnResultMap[pn] = ok;
        sendSqlResp(pn,ok);
    }*/
    bool ok;
    QString sql = QString(get_at_param_string(&ad,1,out_buff,sizeof(out_buff)));
    insertTextToTextBrowser("start query mysql database",TEXT_SQLQUERY);
    insertTextToTextBrowser(SqlHelper::queryMySqlDatabase(sql,ok),TEXT_SQLQUERY);
    sendSqlResp(pn,ok);
}

void DebugDialog::parseAtFWCmd(ATCMD_DATA &ad)
{
    Q_UNUSED(ad);
}

void DebugDialog::parseAtHWCmd(ATCMD_DATA &ad)
{
    Q_UNUSED(ad);
}

void DebugDialog::parseAtERRCmd(ATCMD_DATA &ad)
{
    int errType = -1;
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;

        errType = get_at_param_int(&ad,0);
        if(errType < 1 || errType > 6)
            return;
    }
    else
        return;
    QString str = "AT ERROR";

    switch(errType)
    {
    case 1:
        str += tr(":格式错误");
        break;
    case 2:
        str += tr(":超时错误");
        break;
    case 3:
        str += tr(":设备错误");
        break;
    case 4:
        str += tr(":内存错误");
        break;
    case 5:
        str += tr(":BUSY错误");
        break;
    case 6:
        str += tr(":TIMEOUT错误");
    default:
        break;
    }
    insertTextToTextBrowser(str,TEXT_AT_CMD);
}

void DebugDialog::parseAtOKCmd()
{
    if(isWaitingAtCmdAck())
    {
        m_atTimer->stop();
        switch(m_currentAtCmdType)
        {
        case AT_DOWN:
            break;
        case AT_ISP:
            break;
        case AT_GETPIC:
            break;
        case AT_OTA:
        {
            QString atString = "AT+REST";
            sendAtCmd(atString,AT_REST);
            insertTextToTextBrowser("is resetting device",TEXT_OTA_UPDATE);
            exitCurrentTask();
            break;
        }
        case AT_REST:
            break;
        case AT_MSG:
            break;
        case AT_HEX:
            break;
        default:
            break;
        }
        m_currentAtCmdStr = "";
        m_currentAtCmdType = AT_NULL;
    }
}

void DebugDialog::parseAtHEXCmd(ATCMD_DATA &ad)
{
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_STRING_TYPE)
            return;
    }
    else
        return;

    char str_buff[32*1024];
    char out_buff[32*1024];
    int len;
    get_at_param_string(&ad,0,str_buff,sizeof(str_buff));

    len = HexToBuffer(str_buff ,(unsigned char*)out_buff,sizeof(out_buff));
    if(len >= 0)
    {
        QByteArray data(out_buff,len);
        for(int i = 0;i < data.size();++i)
        {
            if(data[i] == char(0x00))
                data[i] = 0x20;
        }
        QString str = QString(data);
        //str = str.toLatin1();
        str = str.toLocal8Bit();
        insertTextToTextBrowser(str,TEXT_HEX);

    }
    else
        insertTextToTextBrowser(tr("收到的字符串的长度不是偶数"),TEXT_HEX);
}

void DebugDialog::parseAtSIGCmd(ATCMD_DATA &ad)
{
    if(ad.param_count == 1)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;
    }
    else
        return;

    int signal = get_at_param_int(&ad,0);
    //insertTextToTextBrowser(tr("信号强度：%1").arg(signal),TEXT_SIG);
    //emit updateSignal(m_bareJid,signal,0);
}

void DebugDialog::parseAtINFOCmd(ATCMD_DATA &ad)
{
    if(ad.param_count == 2)
    {
        int type0 = ad.param[0].type;
        if(type0 != ATCMD_PARAM_INTEGER_TYPE)
            return;
    }
    else
        return;

    int signal = get_at_param_int(&ad,0);
    int version = get_at_param_int(&ad,1);
    //insertTextToTextBrowser(tr("信号强度：%1").arg(signal),TEXT_SIG);
    emit updateSignal(m_bareJid,signal,version);
}

void DebugDialog::insertTextToTextBrowser(const QString &msg, DebugDialog::BROWSER_TEXT_TYPE type)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("[hh:mm:ss]");

    QString str;
    ui->txb_message->moveCursor(QTextCursor::End);
    switch(type)
    {
    case TEXT_RECV_MESSAGE:
        str = tr("[接受MESSAGE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::blue)));//界面显示
        break;
    case TEXT_SEND_MESSAGE:
        str = tr("[发送MESSAGE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::red)));//界面显示
        break;
    case TEXT_PRESENCE_TEXT:
        str = tr("[接收PRESENCE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::blue)));//界面显示
        break;
    case TEXT_REMOTE_UPDATE:
        str = tr("[REMOTEDOWNLOAD]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));//界面显示
        break;
    case TEXT_LED_UPDATE:
        str = tr("[LEDUPDATE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));//界面显示
        break;
    case TEXT_OTA_UPDATE:
        str = tr("[OTAUPDATE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));//界面显示
        break;
    case TEXT_GETPIC:
        str = tr("[GETPIC]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));//界面显示
        break;
    case TEXT_AT_CMD:
        str = tr("[ATCMD]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));
        break;
    case TEXT_SHOWPIC:
        str = tr("[SHOWPIC]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));
        break;
    case TEXT_SQLQUERY:
        str = tr("[SQLQUERY]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));
        break;
    case TEXT_HEX:
        str = tr("[HEX]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));
        break;
    case TEXT_SENDHEX:
        str = tr("[SENDHEX]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));//界面显示
        break;
    case TEXT_SIG:
        str = tr("[SIGNAL]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));
        break;
    case TEXT_ONLINE:
        str = tr("[ONLINE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));
        break;
    default:
        str = tr("[未知]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(Qt::black)));//界面显示
        break;
    }
    ui->txb_message->moveCursor(QTextCursor::End);

    QString data_msg = dateTime.toString("[yyyy-MM-dd hh:mm:ss]")  + str;
    if(m_client)
        MessageLog::append(m_client->configuration().jidBare(),m_bareJid,data_msg);//保存消息记录
}

void DebugDialog::insertImgToBrowser(const QString &str)
{
    insertTextToTextBrowser(QString("%1:").arg(QFileInfo(str).fileName()),TEXT_SHOWPIC);
    ui->txb_message->moveCursor(QTextCursor::End);
    ui->txb_message->insertHtml(imagePathToHtml(str));
    ui->txb_message->moveCursor(QTextCursor::End);
}

void DebugDialog::createToolMenu()
{
    m_toolMenu = new QMenu(this);
    m_toolMenu->setTitle(tr("工具"));
    this->menuBar()->addMenu(m_toolMenu);

    m_remoteDownloadAction = new QAction(QObject::tr("远程下载"),this);
    connect(m_remoteDownloadAction,SIGNAL(triggered(bool)),SLOT(remoteUpdate()));
    m_toolMenu->addAction(m_remoteDownloadAction);

    m_updateLCDAction = new QAction(QObject::tr("LED更新"),this);
    connect(m_updateLCDAction,SIGNAL(triggered(bool)),SLOT(updateLEDFirmware()));
    m_toolMenu->addAction(m_updateLCDAction);

    m_getPicAction = new QAction(QObject::tr("获取图像"),this);
    connect(m_getPicAction,SIGNAL(triggered(bool)),SLOT(getPic()));
    m_toolMenu->addAction(m_getPicAction);

    m_otaAction = new QAction(QObject::tr("OTA更新"),this);
    connect(m_otaAction,SIGNAL(triggered(bool)),SLOT(otaUpdate()));
    m_toolMenu->addAction(m_otaAction);
}

DebugDialog::DebugDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::debugWindow),m_client(0),m_bareJid(""),m_displayName(""),m_downloadFile(0),
    m_multiPart(0),m_toolMenu(0),m_downloadFileLen(0),m_downloadDev(STM32F10X),m_currentTask(TASK_NULL),m_currentAtCmdStr(""),m_currentAtCmdType(AT_NULL),
    m_online(false)
{
    ui->setupUi(this);

    connect(ui->btn_send,SIGNAL(clicked(bool)),SLOT(sendMessage()));
    connect(ui->btn_hex_send,SIGNAL(clicked(bool)),SLOT(sendHexMessage()));

    ui->txb_send->setFocus();
    ui->pgb_download->hide();
    createToolMenu();


    m_manager = new QNetworkAccessManager(this);

    m_taskTimer = new QTimer;
    m_taskTimer->setSingleShot(true);
    connect(m_taskTimer,SIGNAL(timeout()),SLOT(taskTimeout()));

    m_atTimer = new QTimer;
    m_atTimer->setSingleShot(true);
    connect(m_atTimer,SIGNAL(timeout()),SLOT(atCmdTimeout()));

    m_querySigTimer = new QTimer;
    connect(m_querySigTimer,SIGNAL(timeout()),SLOT(querySignal()));


    m_selectFileDialog = new QFileDialog(this);
    m_selectFileDialog->setWindowModality(Qt::WindowModal);
    m_selectFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    connect(m_selectFileDialog,SIGNAL(fileSelected(QString)),SLOT(fileSelected(QString)));

}

DebugDialog::~DebugDialog()
{
    delete ui;
    m_manager->deleteLater();
    if(m_downloadFile)
    {
        m_downloadFile->close();
        m_downloadFile->deleteLater();
    }
    if(m_multiPart)
        m_multiPart->deleteLater();
    //if(m_remoteDownloadTimer->isActive())
        //m_remoteDownloadTimer->stop();
    //m_remoteDownloadTimer->deleteLater();
    if(m_taskTimer->isActive())
        m_taskTimer->stop();
    m_taskTimer->deleteLater();
    m_updateLCDAction->deleteLater();
    m_toolMenu->deleteLater();
}

void DebugDialog::show()
{
    QMainWindow::show();
    ui->txb_send->setFocus();
}

QString DebugDialog::getBareJid() const
{
    return m_bareJid;
}

QString DebugDialog::getDisplayName() const
{
    return m_displayName;
}

void DebugDialog::setBareJid(const QString &bareJid)
{
    m_bareJid = bareJid;
}

void DebugDialog::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
    this->setWindowTitle(m_displayName);
}

void DebugDialog::setResource(const QStringList &resources)
{
    Q_UNUSED(resources);
    /*ui->cmb_resource->clear();
    for(int i = 0;i < resources.size();++i)
    {
        ui->cmb_resource->insertItem(i,resources[i]);
    }*/
}

void DebugDialog::setQXmppClient(QXmppClient *client)
{
    m_client = client;
}

void DebugDialog::messageReceived(const QXmppMessage &msg)
{
    insertTextToTextBrowser(msg.body(),TEXT_RECV_MESSAGE);//界面显示

    parseAtCmdString(msg.body());//解析AT指令
}

void DebugDialog::presenceReceived(const QXmppPresence &presence)
{
    insertTextToTextBrowser(presence.statusText(),TEXT_PRESENCE_TEXT);//界面显示

    parseAtCmdString(presence.statusText());//解析AT指令
}

void DebugDialog::setBareJidOnline(bool online)
{
    m_online = online;
    if(m_online)
    {
        insertTextToTextBrowser(tr("online"),TEXT_ONLINE);

    }
    else
    {
        insertTextToTextBrowser(tr("offline"),TEXT_ONLINE);
    }
}

void DebugDialog::sendMessage()
{
    if(ui->txb_send->toPlainText().isEmpty())
        return;
    if(m_client)
        m_client->sendMessage(m_bareJid,ui->txb_send->toPlainText());

    insertTextToTextBrowser(ui->txb_send->toPlainText(),TEXT_SEND_MESSAGE);

    ui->txb_send->clear();
}

void DebugDialog::sendHexMessage()
{
    if(ui->txb_send->toPlainText().isEmpty())
        return;
    if(m_client)
    {

        QString hexStr = ui->txb_send->toPlainText().simplified();
        QRegExp reg(QString("\\b[0-9a-fA-F]+\\b"));
        if(hexStr.length()%2 ==0 && reg.exactMatch(hexStr))
        {
            hexStr = hexStr.toUpper();
            QString atCmd = QString("AT+HEX=\"%1\"").arg(hexStr);
            sendAtCmd(atCmd,AT_HEX);
        }
        else
        {
            insertTextToTextBrowser("not the correct hex string",TEXT_SENDHEX);
        }
    }

}

void DebugDialog::sendSqlResp(int pn, bool ok)
{
    QString atCmd;
    if(ok)
        atCmd = QString("AT+SQLRESP=%1,1").arg(pn);
    else
        atCmd = QString("AT+SQLRESP=%1,0").arg(pn);
    sendAtCmd(atCmd,AT_SQLRESP,1);
}

void DebugDialog::httpUpload(const QString &fileName)
{
    insertTextToTextBrowser(tr("filename:%1").arg(fileName),getBrowserTextType(m_currentTask));
    m_downloadFile = new QFile(fileName);
    if(!m_downloadFile->open(QIODevice::ReadOnly))
    {
        insertTextToTextBrowser("file open failed",getBrowserTextType(m_currentTask));
        return;
    }
    m_downloadFileLen = m_downloadFile->size();
    m_fileMD5 = QCryptographicHash::hash(m_downloadFile->readAll(),QCryptographicHash::Md5).toHex().constData();//计算文件MD5
    insertTextToTextBrowser(tr("file md5:%1").arg(m_fileMD5),getBrowserTextType(m_currentTask));

    m_downloadFile->reset();

    //设置HTTP包头-----------------------------------------------------------------------------------------------------------------------------------------
    QNetworkRequest request(QUrl(IniConfig::getFirmwareServer(m_client->configuration().jidBare())));//"http://115.28.44.147/upload2/upload.php"
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data;boundary=1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik9ol0p");

    m_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    m_multiPart->setBoundary(QByteArray("1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik9ol0p"));

    QHttpPart imagePart;
    imagePart.setRawHeader(QByteArray("Content-Type"), QByteArray("application/octet-stream"));
    imagePart.setRawHeader(QByteArray("Content-Disposition"), tr("form-data;name=\"pic\";filename=\"%1\"").arg(fileName.split('/').last()).toLatin1());
    imagePart.setBodyDevice(m_downloadFile);

    m_downloadFile->setParent(m_multiPart); // we cannot delete the file now, so delete it with the multiPart
    m_multiPart->append(imagePart);
    //------------------------------------------------------------------------------------------------------------------------------------------------------

    QNetworkReply* reply = m_manager->post(request,m_multiPart);

    bool check;

    check = connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(httpUploadReplayError(QNetworkReply::NetworkError)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(uploadProgress(qint64,qint64)),SLOT(httpUploadProgress(qint64, qint64)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(finished()),SLOT(httpUploadFinish()));
    Q_ASSERT(check);
}

void DebugDialog::httpUploadReplayError(QNetworkReply::NetworkError error)
{
    insertTextToTextBrowser(tr("upload to server error:%1").arg(error),getBrowserTextType(m_currentTask));
    exitCurrentTask();
}

void DebugDialog::httpUploadProgress(qint64 sendSize, qint64 totalSize)
{
    //m_remoteDownloadTimer->start(HTTP_TIMEROUT);
    m_taskTimer->start(TASK_TIMEOUT);
    if(totalSize > 0)
    {
        ui->pgb_download->show();
        int percentage = sendSize*100/totalSize;
        ui->pgb_download->setValue(percentage);
        ui->pgb_download->show();
        insertTextToTextBrowser(tr("upload to server:%1").arg(percentage),getBrowserTextType(m_currentTask));
    }
}

void DebugDialog::httpDownloadProgress(qint64 recvSize, qint64 totalSize)
{
    m_taskTimer->start(TASK_TIMEOUT);
    if(totalSize > 0)
    {
        ui->pgb_download->show();
        int percentage = recvSize*100/totalSize;
        ui->pgb_download->setValue(percentage);
        ui->pgb_download->show();
        insertTextToTextBrowser(tr("download file:%1").arg(percentage),getBrowserTextType(m_currentTask));
    }
}

void DebugDialog::httpUploadFinish()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    QString result(arr);
    insertTextToTextBrowser(tr("upload to server:%1").arg(QString(arr)),getBrowserTextType(m_currentTask));

    reply->deleteLater();
    m_downloadFile->close();
    m_downloadFile->deleteLater();
    m_downloadFile = 0;
    m_multiPart->deleteLater();
    m_multiPart = 0;
    if(!result.contains("./upload/"))//上传失败
    {
        insertTextToTextBrowser("upload to server failed",getBrowserTextType(m_currentTask));
        m_taskTimer->start(TASK_TIMEOUT);
        return;
    }
    QString atString = tr("AT+DOWN=\"http://www.easy-iot.cc/upload2/upload/%1").arg(result.split('/').last())+'\"';
    sendAtCmd(atString,AT_DOWN);
    ui->pgb_download->setValue(0);
    m_taskTimer->start(TASK_TIMEOUT);
    //qDebug()<<__func__<<m_currentTask;
}

void DebugDialog::taskTimeout()
{
    ui->pgb_download->hide();
    m_toolMenu->setDisabled(false);

    switch(m_currentTask)
    {
    case TASK_GET_PIC:
        insertTextToTextBrowser("get picture timeout",TEXT_GETPIC);
        break;
    case TASK_REMOTE_UPDATE:
        insertTextToTextBrowser("remote update timeout",TEXT_REMOTE_UPDATE);
        break;
    case TASK_LED_UPDATE:
        insertTextToTextBrowser("led update timeout",TEXT_LED_UPDATE);
        break;
    case TASK_OTA_UPDATE:
        insertTextToTextBrowser("ota update timeout",TEXT_OTA_UPDATE);
        break;
    case TASK_SHOW_PIC:
        insertTextToTextBrowser("show picture timeout",TEXT_SHOWPIC);
        break;
    default:
        break;
    }
    m_currentTask = TASK_NULL;
}

void DebugDialog::atCmdTimeout()
{
    insertTextToTextBrowser(QString("atcmd timeout:%1").arg(m_currentAtCmdStr),TEXT_AT_CMD);
    m_currentAtCmdStr = "";
    m_currentAtCmdType = AT_NULL;
}

void DebugDialog::remoteUpdate()
{
    /*QString resource = getDownloadResource();
    if(resource.isEmpty())
    {
        insertTextToTextBrowser("resource is empty",REMOTEDOWNLOAD);
        return;
    }
    insertTextToTextBrowser(tr("resource:%1").arg(resource),REMOTEDOWNLOAD);
    */
    if(m_currentTask != TASK_NULL)
    {
        insertTextToTextBrowser("is performing other task",TEXT_REMOTE_UPDATE);
        return;
    }
    QStringList devList;
    devList.append("STM32F103X");
    devList.append("AVR Mega328/Arduino");
    bool ok;
    QString dev = QInputDialog::getItem(this,tr("选择设备型号"),tr("设备型号："),devList,0,false,&ok);
    if(!ok)
        return;

    if(dev == "STM32F103X")
        m_downloadDev = STM32F10X;
    else if(dev == "AVR Mega328/Arduino")
        m_downloadDev = AVR;
    else
        return;

    if(m_downloadDev == STM32F10X)
        m_selectFileDialog->setWindowTitle(tr("STM32固件"));
    else if(m_downloadDev == AVR)
        m_selectFileDialog->setWindowTitle(tr("AVR固件"));
    else
        m_selectFileDialog->setWindowTitle(tr("固件"));

    m_selectFileDialog->setDirectory(IniConfig::getRemoteDownloadPath(m_client->configuration().jidBare()));

    if(m_downloadDev == STM32F10X)
        m_selectFileDialog->setNameFilter("BIN(*.bin)");
    else
        m_selectFileDialog->setNameFilter("BIN(*.bin)");

    m_selectFileDialog->open();
    m_currentTask = TASK_REMOTE_UPDATE;

//    if(!fileName.isEmpty())
//        IniConfig::setRemoteDownloadPath(fileName,m_client->configuration().jidBare());
//    return fileName;

//    if(fileName.isEmpty())
//        return;

//    m_toolMenu->setDisabled(true);
//    m_currentTask = TASK_REMOTE_UPDATE;
//    if(m_downloadDev == AVR)
//        turnHex2Bin(fileName);
//    else
//    {
//        httpUpload(fileName);
//    }
//    m_taskTimer->start(TASK_TIMEOUT);
}

void DebugDialog::updateLEDFirmware()
{
    if(m_currentTask != TASK_NULL)
    {
        insertTextToTextBrowser("is performing other task",TEXT_REMOTE_UPDATE);
        return;
    }
//    QString ledFileName = getLedFileName();

//    QString fileName = QFileDialog::getOpenFileName(this,tr("LED固件"),,"BIN(*.*)");
//    IniConfig::setLedFilePath(fileName,m_client->configuration().jidBare());
//    return fileName;

    m_selectFileDialog->setWindowTitle(tr("LED固件"));
    m_selectFileDialog->setDirectory(IniConfig::getLedFilePath(m_client->configuration().jidBare()));
    m_selectFileDialog->setNameFilter("BIN(*.bin)");
    m_selectFileDialog->open();
    m_currentTask = TASK_LED_UPDATE;

//    if(ledFileName.isEmpty())
//    {
//        insertTextToTextBrowser("'led firmware' filename is empty",TEXT_LED_UPDATE);
//        return;
//    }
//    m_ledFirmwareFile.setFileName(ledFileName);
//    m_toolMenu->setDisabled(true);

//    geLEDTemplateFirmware();
}

void DebugDialog::getPic()
{
    if(m_currentTask != TASK_NULL)
    {
        insertTextToTextBrowser("is performing other task",TEXT_REMOTE_UPDATE);
        return;
    }
    QStringList deviceList;
    deviceList.append(QObject::tr("VC0706串口摄像头"));
    deviceList.append(QObject::tr("板载OV2640"));
    deviceList.append(QObject::tr("板载OV7670"));
    bool ok;
    QString deviceType = QInputDialog::getItem(this,tr("选择图片源"),tr("图片源:"),deviceList,0,false,&ok);
    if(!ok)
        return;

    QString atString;
    if(deviceType == QObject::tr("VC0706串口摄像头"))
        atString = "AT+GETPIC=0";
    else if(deviceType == QObject::tr("板载OV2640"))
        atString = "AT+GETPIC=1";
    else if(deviceType == QObject::tr("板载OV7670"))
        atString = "AT+GETPIC=2";
    else
        return;

    m_currentTask = TASK_GET_PIC;
    sendAtCmd(atString,AT_GETPIC);
    ui->pgb_download->setValue(0);
    ui->pgb_download->show();
    m_toolMenu->setDisabled(true);
    m_taskTimer->start(TASK_TIMEOUT);
}

void DebugDialog::otaUpdate()
{
    if(m_currentTask != TASK_NULL)
    {
        insertTextToTextBrowser("is performing other task",TEXT_REMOTE_UPDATE);
        return;
    }
   // QString fileName = QFileDialog::getOpenFileName(this,tr("选择需要OTA更新的文件"),IniConfig::getOtaFilePath(m_client->configuration().jidBare()),"*.bin");

    m_selectFileDialog->setWindowTitle(tr("选择需要OTA更新的文件"));
    m_selectFileDialog->setDirectory(IniConfig::getOtaFilePath(m_client->configuration().jidBare()));
    m_selectFileDialog->setNameFilter("BIN(*.bin)");
    m_selectFileDialog->open();
    m_currentTask = TASK_OTA_UPDATE;

//    if(fileName.isEmpty())
//        return;
//    IniConfig::setOtaFilePath(fileName,m_client->configuration().jidBare());

//    m_toolMenu->setDisabled(true);
//    m_currentTask = TASK_OTA_UPDATE;
//    httpUpload(fileName);
//    m_taskTimer->start(TASK_TIMEOUT);
}

void DebugDialog::geLEDTemplateFirmware()
{
    if(m_ledTemplateFirmwareFile.isOpen())
    {
        insertTextToTextBrowser("template file already opened",TEXT_LED_UPDATE);
        m_toolMenu->setDisabled(false);
        return;
    }

    QString fileName;
    if(m_client)
        fileName = IniConfig::getLedTemplateFilePath(m_client->configuration().jidBare())+"ledtemplate.bin";

    m_ledTemplateFirmwareFile.remove(fileName);
    m_ledTemplateFirmwareFile.setFileName(fileName);
    if(!m_ledTemplateFirmwareFile.open(QIODevice::ReadWrite))
    {
        insertTextToTextBrowser("template file open failed",TEXT_LED_UPDATE);
        m_toolMenu->setDisabled(false);
        return;
    }

    insertTextToTextBrowser("download template file",TEXT_LED_UPDATE);
    QNetworkReply * reply = m_manager->get(QNetworkRequest(QUrl(IniConfig::getLedTemplateFilePath(m_client->configuration().jidBare()))));//"http://www.easy-iot.cc/download/easyled_firmware.bin"

    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(httpDownloadProgress(qint64,qint64)));
    connect(reply,SIGNAL(readyRead()),SLOT(readHttpData()));
    connect(reply,SIGNAL(finished()),SLOT(httpDownloadFinish()));
    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),SLOT(httpDownloadError(QNetworkReply::NetworkError)));

    m_taskTimer->start(TASK_TIMEOUT);
}

void DebugDialog::exitCurrentTask()
{
    m_taskTimer->stop();
    m_currentTask = TASK_NULL;
    ui->pgb_download->hide();
    m_toolMenu->setDisabled(false);
}

DebugDialog::BROWSER_TEXT_TYPE DebugDialog::getBrowserTextType(DebugDialog::TASK task)
{
    BROWSER_TEXT_TYPE text_type = TEXT_UNKNOW;
    switch (task) {
    case TASK_REMOTE_UPDATE:
        text_type = TEXT_REMOTE_UPDATE;
        break;
    case TASK_LED_UPDATE:
        text_type = TEXT_LED_UPDATE;
        break;
    case TASK_OTA_UPDATE:
        text_type = TEXT_OTA_UPDATE;
        break;
    case TASK_GET_PIC:
        text_type = TEXT_GETPIC;
        break;
    case TASK_SHOW_PIC:
        text_type = TEXT_SHOWPIC;
    default:
        break;
    }
    return text_type;
}

void DebugDialog::turnHex2Bin(const QString &fileName)
{
    m_turnFileName = fileName;
    m_turnHexProcess = new QProcess;
    QString programPath = "hex2bin.exe";
    QStringList paramList;
    paramList.append(programPath);
    paramList.append(fileName);
    connect(m_turnHexProcess,SIGNAL(error(QProcess::ProcessError)),SLOT(turnHexProcessError(QProcess::ProcessError)));
    connect(m_turnHexProcess,SIGNAL(finished(int)),SLOT(turnHexFinished(int)));
    m_turnHexProcess->start(programPath,paramList);
}

QString DebugDialog::turnFileType2Bin(const QString &filePathName)
{

    QString path = QFileInfo(filePathName).path()+'/';
    QString fileName = QFileInfo(filePathName).fileName();
    qDebug()<<path<<fileName;
    QStringList strList = fileName.split('.');
    if(strList.size()>1)
        strList.removeLast();
    fileName = "";
    for(int i = 0;i < strList.size();++i)
        fileName += strList[i]+'.';
    fileName += "bin";

    return path+fileName;
}

void DebugDialog::readHttpData()
{
    m_taskTimer->start(TASK_TIMEOUT);
    QNetworkReply* reply = (QNetworkReply*)this->sender();
    if(m_ledTemplateFirmwareFile.isOpen())
    {
        m_ledTemplateFirmwareFile.write(reply->readAll());
    }
}

void DebugDialog::turnHexProcessError(QProcess::ProcessError)
{
    insertTextToTextBrowser("turn hex file to bin file failed",getBrowserTextType(m_currentTask));
    exitCurrentTask();
}

void DebugDialog::turnHexFinished(int ret)
{
    if(m_currentTask != TASK_NULL)
    {
        qDebug()<<"process return:"<<ret;
        m_taskTimer->start(TASK_TIMEOUT);
        insertTextToTextBrowser("turn hex file to bin file successed",getBrowserTextType(m_currentTask));
        httpUpload(turnFileType2Bin(m_turnFileName));
    }
    m_turnHexProcess->deleteLater();
}

void DebugDialog::waitTurnFileCloseTimeout()
{
    insertTextToTextBrowser("start upload file",getBrowserTextType(m_currentTask));
    httpUpload(turnFileType2Bin(m_turnFileName));
    m_waitTurnFileCloseTimer->deleteLater();
}

void DebugDialog::querySignal()
{
    QString atStr = "AT+SIG";
    sendAtCmd(atStr,AT_QUERY_SIGNAL);
}

void DebugDialog::fileSelected(const QString &fileName)
{
    if(m_currentTask == TASK_REMOTE_UPDATE)
    {
        if(!fileName.isEmpty())
            IniConfig::setRemoteDownloadPath(fileName,m_client->configuration().jidBare());
        else
        {
            m_currentTask = TASK_NULL;
            return;
        }

        m_toolMenu->setDisabled(true);

        if(m_downloadDev == AVR)
            turnHex2Bin(fileName);
        else
        {
            httpUpload(fileName);
        }
        m_taskTimer->start(TASK_TIMEOUT);
    }
    else if(m_currentTask == TASK_LED_UPDATE)
    {
        if(fileName.isEmpty())
        {
            insertTextToTextBrowser("'led firmware' filename is empty",TEXT_LED_UPDATE);
            return;
        }
        IniConfig::setLedFilePath(fileName,m_client->configuration().jidBare());
        m_ledFirmwareFile.setFileName(fileName);
        m_toolMenu->setDisabled(true);

        geLEDTemplateFirmware();
    }
    else if(m_currentTask == TASK_OTA_UPDATE)
    {
        if(fileName.isEmpty())
            return;
        IniConfig::setOtaFilePath(fileName,m_client->configuration().jidBare());

        m_toolMenu->setDisabled(true);
        m_currentTask = TASK_OTA_UPDATE;
        httpUpload(fileName);
        m_taskTimer->start(TASK_TIMEOUT);
    }
}

void DebugDialog::httpDownloadFinish()
{
    QNetworkReply* reply = (QNetworkReply*)this->sender();
    reply->deleteLater();

    insertTextToTextBrowser("template download finished",TEXT_LED_UPDATE);

    QFile outFile;
    if(m_client)
        outFile.setFileName(IniConfig::getLedTemplateFilePath(m_client->configuration().jidBare())+"led.bin");

    if(m_ledTemplateFirmwareFile.isOpen())
        m_ledTemplateFirmwareFile.close();
    if(!m_ledTemplateFirmwareFile.open(QIODevice::ReadOnly))
    {
        insertTextToTextBrowser("led template file open failed",TEXT_LED_UPDATE);
        exitCurrentTask();
        return;
    }
    if(!m_ledFirmwareFile.open(QIODevice::ReadOnly))
    {
        m_ledTemplateFirmwareFile.close();
        insertTextToTextBrowser("led firmware open failed",TEXT_LED_UPDATE);
        exitCurrentTask();
        return;
    }
    outFile.remove(outFile.fileName());
    if(!outFile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        insertTextToTextBrowser("led outfile open failed",TEXT_LED_UPDATE);
        m_ledTemplateFirmwareFile.close();
        m_ledFirmwareFile.close();
        exitCurrentTask();
        return;
    }

    if(m_ledTemplateFirmwareFile.size() < 40*1024)
    {
        insertTextToTextBrowser("led template file's size less than 40KB",TEXT_LED_UPDATE);
        m_ledTemplateFirmwareFile.close();
        m_ledFirmwareFile.close();
        outFile.close();
        exitCurrentTask();
        return;
    }
    else if(m_ledTemplateFirmwareFile.size() > 100*1024)
    {
        insertTextToTextBrowser("led template file's size greater than 100KB",TEXT_LED_UPDATE);
        m_ledTemplateFirmwareFile.close();
        m_ledFirmwareFile.close();
        outFile.close();
        exitCurrentTask();
        return;
    }
    if(m_ledFirmwareFile.size() > 20*1024)
    {
        insertTextToTextBrowser("led firmware file's size greater than 20KB",TEXT_LED_UPDATE);
        m_ledTemplateFirmwareFile.close();
        m_ledFirmwareFile.close();
        outFile.close();
        exitCurrentTask();
        return;
    }
    char buff1[100*1024];
    char buff2[20*1024];

    unsigned len1,len2;
    len1 = m_ledTemplateFirmwareFile.size();
    m_ledTemplateFirmwareFile.read(buff1,len1);
    len2 = m_ledFirmwareFile.size();
    m_ledFirmwareFile.read(buff2,len2);
    memcpy(buff1+20*1024,&len2,4);
    memcpy(buff1+20*1024+4,buff2,len2);
    outFile.write(buff1,len1);

    m_ledTemplateFirmwareFile.close();
    m_ledFirmwareFile.close();
    outFile.close();

    insertTextToTextBrowser("combin finished",TEXT_LED_UPDATE);
    m_downloadDev = STM32F10X;
    httpUpload(outFile.fileName());
    m_taskTimer->start(TASK_TIMEOUT);
    qDebug()<<TASK_LED_UPDATE<<m_currentTask;

}

void DebugDialog::httpDownloadError(QNetworkReply::NetworkError error)
{
    insertTextToTextBrowser(QString("http download error:%1").arg(error),TEXT_LED_UPDATE);
    exitCurrentTask();
}

void DebugDialog::imageDownloadFinish()
{

    QNetworkReply *reply = (QNetworkReply*)sender();
    QByteArray data = reply->readAll();
    if(data.size() == 0)
    {
        insertTextToTextBrowser("download failed",TEXT_SHOWPIC);
        return;
    }
    else
        insertTextToTextBrowser("download successed",TEXT_SHOWPIC);

    QString imageName = reply->url().fileName();

    QPixmap pixmap;
    pixmap.loadFromData(data);

    QString fileName = IniConfig::getSaveImageFilePath(m_client->configuration().jidBare(),m_bareJid)+imageName;
    pixmap.save(fileName);

    insertImgToBrowser(fileName);
    reply->deleteLater();
    exitCurrentTask();
}

void DebugDialog::imageDownloadError(QNetworkReply::NetworkError error)
{
    insertTextToTextBrowser(QString("image download error:%1").arg(error),TEXT_SHOWPIC);
    exitCurrentTask();
}

void DebugDialog::keyPressEvent(QKeyEvent* event)
{
    QMainWindow::keyPressEvent(event);
    ui->txb_send->setFocus();
    if(event->key() == Qt::Key_Return)
    {
        sendMessage();
    }
    else if(event->key() == Qt::Key_Escape)
    {
        hide();
    }
}
