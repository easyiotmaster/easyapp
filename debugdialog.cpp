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

QString DebugDialog::getDownloadFileName()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("固件"),IniConfig::getRemoteDownloadPath(m_client->configuration().jidBare()),"BIN(*.bin)");
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

int DebugDialog::parseRemoteDownloadACK(const QString &msg)
{
    if(msg == "ERROR")
    {
        switch(m_lastDownloadAction)
        {
            case AT_DOWN:
                insertTextToTextBrowser("AT DOWN ERROR",REMOTEDOWNLOAD);
                break;
            case AT_ISP:
                insertTextToTextBrowser("AT ISP ERROR",REMOTEDOWNLOAD);
                break;
            case AT_RST:
                insertTextToTextBrowser("AT RST ERROR",REMOTEDOWNLOAD);
                break;
            default:
                return 0;
        }
        ui->pgb_download->hide();
        m_toolMenu->setDisabled(false);
    }
    else if(msg == "OK")
    {

        switch(m_lastDownloadAction)
        {
            case AT_DOWN:
                ui->pgb_download->setValue(0);
                break;
            case AT_ISP:
                ui->pgb_download->setValue(0);
                break;
            case AT_RST:
                insertTextToTextBrowser("RST SUCCESSED",REMOTEDOWNLOAD);
                ui->pgb_download->setValue(0);
                ui->pgb_download->hide();
                m_toolMenu->setDisabled(false);
                break;
            default:
                return 0;
        }


    }
    else if(msg.contains("+DOWN=ERROR"))
    {
        ui->pgb_download->hide();
        m_toolMenu->setDisabled(false);
    }
    else if(msg.contains("+DOWN=MD5"))
    {
        ui->pgb_download->setValue(100);
        QStringList md5List = msg.split(',');
        QString md5;
        if(md5List.size() != 2)
        {
            insertTextToTextBrowser("MD5 parse error",REMOTEDOWNLOAD);
            ui->pgb_download->hide();
            m_toolMenu->setDisabled(false);
        }
        else
        {
            md5 = md5List[1];
            if(md5 != m_fileMD5)
            {
                insertTextToTextBrowser("MD5 does not match",REMOTEDOWNLOAD);
                ui->pgb_download->hide();
                m_toolMenu->setDisabled(false);
            }
            else
            {
                QString atString = tr("at+isp");
                m_client->sendMessage(m_bareJid,atString);
                insertTextToTextBrowser(atString,SEND_MESSAGE);
                m_lastDownloadAction = AT_ISP;
                //lockUI(false);
            }
        }

    }
    else if(msg.contains("+DOWN="))
    {
        QStringList strList = msg.split('=');
        QString downSizeStr,totalSizeStr;
        if(strList.size() != 2)
        {
            insertTextToTextBrowser("MD5 parse error",REMOTEDOWNLOAD);
            ui->pgb_download->hide();
            m_toolMenu->setDisabled(false);
            return 1;
        }
        else
        {
            strList = strList[1].split(',');
            if(strList.size() != 2)
            {
                insertTextToTextBrowser("MD5 parse error",REMOTEDOWNLOAD);
                ui->pgb_download->hide();
                m_toolMenu->setDisabled(false);
                return 1;
            }
            else
            {
                downSizeStr = strList[0];
                totalSizeStr = strList[1];
            }

        }
        qDebug()<<downSizeStr<<totalSizeStr;
        quint64 downSize = downSizeStr.toLong();
        quint64 totalSize = totalSizeStr.toLong();
        if(totalSize != 0)
        {
            int percentage = downSize*100/totalSize;
            ui->pgb_download->setValue(percentage);
        }
    }
    else if(msg.contains("+ISP=SUCCESS"))
    {
        ui->pgb_download->setValue(100);
        insertTextToTextBrowser("remote download success",REMOTEDOWNLOAD);
//        QString atString = tr("at+rst");
//        m_client->sendMessage(m_barejid,atString);
//        ui->txd_downloadlog->append(tr("[AT]%1").arg(atString));
//        m_lastDownloadAction = AT_RST;
        ui->pgb_download->hide();
        m_toolMenu->setDisabled(false);
        m_remoteDownloadTimer->stop();
        return 2;
    }
    else if(msg.contains("+ISP=ERROR"))
    {
        ui->pgb_download->hide();
        m_toolMenu->setDisabled(false);
    }
    else if(msg.contains("+ISP="))
    {
        QStringList strList = msg.split('=');
        QString downSizeStr,totalSizeStr;
        if(strList.size() != 2)
        {
            insertTextToTextBrowser("isp parse error",REMOTEDOWNLOAD);
            ui->pgb_download->hide();
            m_toolMenu->setDisabled(false);
            return 1;
        }
        else
        {
            strList = strList[1].split(',');
            if(strList.size() != 2)
            {
                insertTextToTextBrowser("isp parse error",REMOTEDOWNLOAD);
                ui->pgb_download->hide();
                m_toolMenu->setDisabled(false);
                return 1;
            }
            else
            {
                downSizeStr = strList[0];
                totalSizeStr = strList[1];
            }

        }
        quint64 downSize = downSizeStr.toLong();
        quint64 totalSize = totalSizeStr.toLong();
        if(totalSize != 0)
        {
            int percentage = downSize*100/totalSize;
            ui->pgb_download->setValue(percentage);
        }
    }
    else if(msg == "Handshake success!")
    {

    }
    else if(msg == "CONNECTED ! START PROGRAM .")
    {

    }
    else
    {
        return 0;
    }
    return 1;
}

void DebugDialog::insertTextToTextBrowser(const QString &msg, DebugDialog::BROWSER_TEXT_TYPE type)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("[hh:mm:ss]");

    QString str;
    ui->txb_message->moveCursor(QTextCursor::End);
    switch(type)
    {
    case RECV_MESSAGE:
        str = tr("[接受MESSAGE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(0,0,255)));//界面显示
        break;
    case SEND_MESSAGE:
        str = tr("[发送MESSAGE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(255,0,0)));//界面显示
        break;
    case PRESENCE_TEXT:
        str = tr("[接收PRESENCE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(0,255,0)));//界面显示
        break;
    case REMOTEDOWNLOAD:
        str = tr("[REMOTEDOWNLOAD]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(0,255,255)));//界面显示
        break;
    case LEDUPDATE:
        str = tr("[LEDUPDATE]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(0,255,255)));//界面显示
        break;
    default:
        str = tr("[未知]:") + msg;
        ui->txb_message->insertHtml(stringToHtml(time + str,QColor(0,0,0)));//界面显示
        break;
    }
    ui->txb_message->moveCursor(QTextCursor::End);

    QString data_msg = dateTime.toString("[yyyy-MM-dd hh:mm:ss]")  + str;
    if(m_client)
        MessageLog::append(m_client->configuration().jidBare(),m_bareJid,data_msg);//保存消息记录
}

void DebugDialog::createToolMenu()
{
    m_toolMenu = new QMenu(this);
    m_toolMenu->setTitle(tr("工具"));
    this->menuBar()->addMenu(m_toolMenu);

    m_remoteDownloadAction = new QAction(QObject::tr("远程下载"),this);
    connect(m_remoteDownloadAction,SIGNAL(triggered(bool)),SLOT(remoteDownload()));
    m_toolMenu->addAction(m_remoteDownloadAction);

    m_updateLCDAction = new QAction(QObject::tr("LED更新"),this);
    connect(m_updateLCDAction,SIGNAL(triggered(bool)),SLOT(updateLEDFirmware()));
    m_toolMenu->addAction(m_updateLCDAction);
}

DebugDialog::DebugDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::debugWindow),m_client(0),m_bareJid(""),m_displayName(""),m_downloadFile(0),m_multiPart(0),m_toolMenu(0)
{
    ui->setupUi(this);
    connect(ui->btn_send,SIGNAL(clicked(bool)),SLOT(sendMessage()));
    ui->txb_send->setFocus();

    ui->pgb_download->hide();
    createToolMenu();


    m_manager = new QNetworkAccessManager(this);

    m_remoteDownloadTimer = new QTimer;
    m_remoteDownloadTimer->setSingleShot(true);
    connect(m_remoteDownloadTimer,SIGNAL(timeout()),SLOT(remoteDownloadTimeout()));

    m_httpDownloadTimer = new QTimer;
    m_httpDownloadTimer->setSingleShot(true);
    connect(m_httpDownloadTimer,SIGNAL(timeout()),SLOT(httpDownloadTimeout()));
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
    if(m_remoteDownloadTimer->isActive())
        m_remoteDownloadTimer->stop();
    m_remoteDownloadTimer->deleteLater();
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

void DebugDialog::setResource(const QStringList &resouces)
{
    /*ui->cmb_resource->clear();
    for(int i = 0;i < resouces.size();++i)
    {
        ui->cmb_resource->insertItem(i,resouces[i]);
    }*/
}

void DebugDialog::setQXmppClient(QXmppClient *client)
{
    m_client = client;
}

void DebugDialog::messageReceived(const QXmppMessage &msg)
{
    insertTextToTextBrowser(msg.body(),RECV_MESSAGE);

    if(parseRemoteDownloadACK(msg.body()) == 1)
        m_remoteDownloadTimer->start(ATTIMEOUT);

}

void DebugDialog::presenceReceived(const QXmppPresence &presence)
{
    insertTextToTextBrowser(presence.statusText(),PRESENCE_TEXT);
}

void DebugDialog::sendMessage()
{
    if(ui->txb_send->toPlainText().isEmpty())
        return;
    if(m_client)
        m_client->sendMessage(m_bareJid,ui->txb_send->toPlainText());

    insertTextToTextBrowser(ui->txb_send->toPlainText(),SEND_MESSAGE);

    ui->txb_send->clear();
}

void DebugDialog::remoteDownload()
{
    /*QString resource = getDownloadResource();
    if(resource.isEmpty())
    {
        insertTextToTextBrowser("resource is empty",REMOTEDOWNLOAD);
        return;
    }
    insertTextToTextBrowser(tr("resource:%1").arg(resource),REMOTEDOWNLOAD);
    */

    QString fileName = getDownloadFileName();
    if(fileName.isEmpty())
    {
        insertTextToTextBrowser("filename is empty",REMOTEDOWNLOAD);
        return;
    }
    //ui->btn_remotedownload->setEnabled(false);
    m_toolMenu->setDisabled(true);
    remoteDownload(fileName);
}

void DebugDialog::remoteDownload(const QString &fileName)
{
    insertTextToTextBrowser(tr("filename:%1").arg(fileName),REMOTEDOWNLOAD);
    m_downloadFile = new QFile(fileName);
    if(!m_downloadFile->open(QIODevice::ReadOnly))
    {
        insertTextToTextBrowser("file open failed",REMOTEDOWNLOAD);
        return;
    }

    m_fileMD5 = QCryptographicHash::hash(m_downloadFile->readAll(),QCryptographicHash::Md5).toHex().constData();//计算文件MD5
    insertTextToTextBrowser(tr("file md5:%1").arg(m_fileMD5),REMOTEDOWNLOAD);

    m_downloadFile->reset();

    //设置HTTP包头-----------------------------------------------------------------------------------------------------------------------------------------
    QNetworkRequest request(QUrl("http://115.28.44.147/upload2/upload.php"));
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

    check = connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(remoteDownloadReplayError(QNetworkReply::NetworkError)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(uploadProgress(qint64,qint64)),SLOT(uploadProgress(qint64, qint64)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(finished()),SLOT(uploadFinish()));
    Q_ASSERT(check);

    m_lastDownloadAction = UPLOAD_TO_HTTPSERVER;
    m_remoteDownloadTimer->start(10000);
}



void DebugDialog::remoteDownloadReplayError(QNetworkReply::NetworkError error)
{
    insertTextToTextBrowser(tr("upload to server error:%1").arg(error),REMOTEDOWNLOAD);
    m_toolMenu->setDisabled(false);
    ui->pgb_download->hide();
}


void DebugDialog::uploadProgress(qint64 sendSize, qint64 totalSize)
{
    m_remoteDownloadTimer->start(HTTPTIMEROUT);
    if(totalSize > 0)
    {
        ui->pgb_download->show();
        int percentage = sendSize*100/totalSize;
        ui->pgb_download->setValue(percentage);
        insertTextToTextBrowser(tr("upload to server:%1").arg(percentage),REMOTEDOWNLOAD);
    }
}

void DebugDialog::downloadProgress(qint64 recvSize, qint64 totalSize)
{
    if(totalSize > 0)
    {
        ui->pgb_download->show();
        int percentage = recvSize*100/totalSize;
        ui->pgb_download->setValue(percentage);
        insertTextToTextBrowser(tr("download led template file:%1").arg(percentage),LEDUPDATE);
    }
}

void DebugDialog::uploadFinish()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    QString result(arr);
    insertTextToTextBrowser(tr("upload to server:%1").arg(QString(arr)),REMOTEDOWNLOAD);

    reply->deleteLater();
    m_downloadFile->close();
    m_downloadFile->deleteLater();
    m_downloadFile = 0;
    m_multiPart->deleteLater();
    m_multiPart = 0;
    if(!result.contains("./upload/"))//上传失败
    {
        insertTextToTextBrowser("upload to server failed",REMOTEDOWNLOAD);
        m_toolMenu->setDisabled(false);
        ui->pgb_download->hide();
        return;
    }
    QString atString = tr("at+down=http://115.28.44.147/upload2/upload/%1").arg(result.split('/').last());
    m_client->sendMessage(m_bareJid,atString);
    insertTextToTextBrowser(atString,SEND_MESSAGE);
    m_lastDownloadAction = AT_DOWN;
    m_remoteDownloadTimer->start(HTTPTIMEROUT);
}

void DebugDialog::remoteDownloadTimeout()
{
    switch(m_lastDownloadAction)
    {
        case UPLOAD_TO_HTTPSERVER:
            insertTextToTextBrowser("upload timeout",REMOTEDOWNLOAD);
            break;
        case AT_DOWN:
            insertTextToTextBrowser("AT DOWN timeout",REMOTEDOWNLOAD);
            break;
        case AT_ISP:
            insertTextToTextBrowser("AT ISP timeout",REMOTEDOWNLOAD);
            break;
        case AT_RST:
            insertTextToTextBrowser("AT RST timeout",REMOTEDOWNLOAD);
            break;
        default:
            break;
    }
    ui->pgb_download->setValue(0);
    ui->pgb_download->hide();
    m_toolMenu->setDisabled(false);
}

void DebugDialog::httpDownloadTimeout()
{
    insertTextToTextBrowser("http download timeout",LEDUPDATE);
    ui->pgb_download->setValue(0);
    ui->pgb_download->hide();
    m_toolMenu->setDisabled(false);
}

void DebugDialog::updateLEDFirmware()
{
    QString ledFileName = getLedFileName();
    if(ledFileName.isEmpty())
    {
        insertTextToTextBrowser("'led firmware' filename is empty",LEDUPDATE);
        return;
    }
    m_ledFirmwareFile.setFileName(ledFileName);
    m_toolMenu->setDisabled(true);
    geLEDTemplateFirmware();
}

void DebugDialog::geLEDTemplateFirmware()
{
    if(m_ledTemplateFirmwareFile.isOpen())
    {
        insertTextToTextBrowser("template file already opened",LEDUPDATE);
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
        insertTextToTextBrowser("template file open failed",LEDUPDATE);
        m_toolMenu->setDisabled(false);
        return;
    }

    insertTextToTextBrowser("download template file",LEDUPDATE);
    QNetworkReply * reply = m_manager->get(QNetworkRequest(QUrl("http://www.easy-iot.cc/download/easyled_firmware.bin")));

    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(downloadProgress(qint64,qint64)));
    connect(reply,SIGNAL(readyRead()),SLOT(readHttpData()));
    connect(reply,SIGNAL(finished()),SLOT(httpDownloadFinish()));

    m_httpDownloadTimer->start(HTTPTIMEROUT);
}

void DebugDialog::readHttpData()
{
    m_httpDownloadTimer->start(HTTPTIMEROUT);
    QNetworkReply* reply = (QNetworkReply*)this->sender();
    if(m_ledTemplateFirmwareFile.isOpen())
    {
        m_ledTemplateFirmwareFile.write(reply->readAll());
    }
}

void DebugDialog::httpDownloadFinish()
{
    m_httpDownloadTimer->stop();
    QNetworkReply* reply = (QNetworkReply*)this->sender();
    reply->deleteLater();

    insertTextToTextBrowser("template download finished",LEDUPDATE);

    QFile outFile;
    if(m_client)
        outFile.setFileName(IniConfig::getLedTemplateFilePath(m_client->configuration().jidBare())+"led.bin");

    if(m_ledTemplateFirmwareFile.isOpen())
        m_ledTemplateFirmwareFile.close();
    if(!m_ledTemplateFirmwareFile.open(QIODevice::ReadOnly))
    {
        insertTextToTextBrowser("led template file open failed",LEDUPDATE);
        m_toolMenu->setDisabled(false);
        ui->pgb_download->setValue(0);
        ui->pgb_download->hide();
        return;
    }
    if(!m_ledFirmwareFile.open(QIODevice::ReadOnly))
    {
        m_ledTemplateFirmwareFile.close();
        insertTextToTextBrowser("led firmware open failed",LEDUPDATE);
        m_toolMenu->setDisabled(false);
        ui->pgb_download->setValue(0);
        ui->pgb_download->hide();
        return;
    }
    outFile.remove(outFile.fileName());
    if(!outFile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        insertTextToTextBrowser("led outfile open failed",LEDUPDATE);
        m_ledTemplateFirmwareFile.close();
        m_ledFirmwareFile.close();
        m_toolMenu->setDisabled(false);
        ui->pgb_download->setValue(0);
        ui->pgb_download->hide();
        return;
    }

    if(m_ledTemplateFirmwareFile.size() < 20*1024)
    {
        insertTextToTextBrowser("led template file's size less than 20KB",LEDUPDATE);
        m_ledTemplateFirmwareFile.close();
        m_ledFirmwareFile.close();
        outFile.close();
        m_toolMenu->setDisabled(false);
        ui->pgb_download->setValue(0);
        ui->pgb_download->hide();
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

    /*
    outFile.write(m_ledTemplateFirmwareFile.read(20*1024));

    quint32 ledFileLen = m_ledFirmwareFile.size();
    qDebug()<<ledFileLen;
    char ledFileLenBuff[4];
    memcpy(ledFileLenBuff,&ledFileLen,4);
    qDebug()<<int(ledFileLenBuff[0])<<int(ledFileLenBuff[1])<<int(ledFileLenBuff[2])<<int(ledFileLenBuff[3]);
    outFile.write(ledFileLenBuff,4);

    //QByteArray arr = m_ledFirmwareFile.readAll();
    char ledFileBuf[100*1024];
    qDebug()<<ledFileLen<<m_ledFirmwareFile.read(ledFileBuf,ledFileLen);
    outFile.write(ledFileBuf,ledFileLen);*/

    m_ledTemplateFirmwareFile.close();
    m_ledFirmwareFile.close();
    outFile.close();

    insertTextToTextBrowser("combin finished",LEDUPDATE);
    remoteDownload(outFile.fileName());

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
