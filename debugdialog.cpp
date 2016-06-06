#include "debugdialog.h"
#include "ui_debugdialog.h"
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
    QString fileName = QFileDialog::getOpenFileName(this,tr("选择下载文件"),IniConfig::getRemoteDownloadPath(m_client->configuration().jidBare()),tr("BIN(*.bin)"));
    IniConfig::setRemoteDownloadPath(fileName,m_client->configuration().jidBare());
    return fileName;
}

QString DebugDialog::getDownloadResource()
{
    return ui->cmb_resource->currentText();
}

int DebugDialog::parseRemoteDownloadACK(const QString &msg)
{

    if(msg == "ERROR")
    {
        switch(m_lastDownloadAction)
        {
            case AT_DOWN:
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:AT DOWN ERROR"));
                break;
            case AT_ISP:
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:AT ISP ERROR"));
                break;
            case AT_RST:
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:AT RST ERROR"));
                break;
            default:
                return 0;
        }
        ui->pgb_download->hide();
        ui->btn_remotedownload->setEnabled(true);
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
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:RST SUCCESSED"));
                break;
            default:
                return 0;
        }
        ui->pgb_download->hide();
        ui->btn_remotedownload->setEnabled(true);

    }
    else if(msg.contains("+DOWN=ERROR"))
    {
        ui->pgb_download->hide();
        ui->btn_remotedownload->setEnabled(true);
    }
    else if(msg.contains("+DOWN=MD5"))
    {
        ui->pgb_download->setValue(100);
        QStringList md5List = msg.split(',');
        QString md5;
        if(md5List.size() != 2)
        {
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:MD5 parse error"));
            ui->pgb_download->hide();
            ui->btn_remotedownload->setEnabled(true);
        }
        else
        {
            md5 = md5List[1];
            if(md5 != m_fileMD5)
            {
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:MD5 does not match"));
                ui->pgb_download->hide();
                ui->btn_remotedownload->setEnabled(true);
            }
            else
            {
                QString atString = tr("at+isp");
                m_client->sendMessage(m_bareJid,atString);
                insertSendMessageToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]")+ tr("[发送MESSAGE]:%1").arg(atString));
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
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:MD5 parse error"));
            ui->pgb_download->hide();
            ui->btn_remotedownload->setEnabled(true);
            return 1;
        }
        else
        {
            strList = strList[1].split(',');
            if(strList.size() != 2)
            {
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:MD5 parse error"));
                ui->pgb_download->hide();
                ui->btn_remotedownload->setEnabled(true);
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
    else if(msg.contains("+ISP=SUCCESS"))
    {
        ui->pgb_download->setValue(100);
        insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:remote download success"));
//        QString atString = tr("at+rst");
//        m_client->sendMessage(m_barejid,atString);
//        ui->txd_downloadlog->append(tr("[AT]%1").arg(atString));
//        m_lastDownloadAction = AT_RST;
        ui->pgb_download->hide();
        ui->btn_remotedownload->setEnabled(true);
        m_downTimer->stop();
        return 2;
    }
    else if(msg.contains("+ISP=ERROR"))
    {
        ui->pgb_download->hide();
        ui->btn_remotedownload->setEnabled(true);
    }
    else if(msg.contains("+ISP="))
    {
        QStringList strList = msg.split('=');
        QString downSizeStr,totalSizeStr;
        if(strList.size() != 2)
        {
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:isp parse error"));
            ui->pgb_download->hide();
            ui->btn_remotedownload->setEnabled(true);
            return 1;
        }
        else
        {
            strList = strList[1].split(',');
            if(strList.size() != 2)
            {
                insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:isp parse error"));
                ui->pgb_download->hide();
                ui->btn_remotedownload->setEnabled(true);
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

void DebugDialog::insertRecvMessageToTextBrowser(const QString &msg)
{
    ui->txb_message->moveCursor(QTextCursor::End);
    ui->txb_message->insertHtml(stringToHtml(msg,QColor(0,0,255)));//界面显示
    ui->txb_message->moveCursor(QTextCursor::End);
}

void DebugDialog::insertSendMessageToTextBrowser(const QString &msg)
{
    ui->txb_message->moveCursor(QTextCursor::End);
    ui->txb_message->insertHtml(stringToHtml(msg,QColor(255,0,0)));//界面显示
    ui->txb_message->moveCursor(QTextCursor::End);
}

void DebugDialog::insertPresenceToTextBrowser(const QString &msg)
{
    ui->txb_message->moveCursor(QTextCursor::End);
    ui->txb_message->insertHtml(stringToHtml(msg,QColor(0,255,0)));//界面显示
    ui->txb_message->moveCursor(QTextCursor::End);
}

void DebugDialog::insertParseTextToTextBrowser(const QString &msg)
{
    ui->txb_message->moveCursor(QTextCursor::End);
    ui->txb_message->insertHtml(stringToHtml(msg,QColor(0,255,255)));//界面显示
    ui->txb_message->moveCursor(QTextCursor::End);
}

DebugDialog::DebugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugDialog),m_client(0),m_bareJid(""),m_displayName(""),m_downloadFile(0),m_multiPart(0)
{
    ui->setupUi(this);
    connect(ui->btn_send,SIGNAL(clicked(bool)),SLOT(sendMessage()));
    ui->txb_send->setFocus();

    ui->pgb_download->hide();

    m_manager = new QNetworkAccessManager(this);
    //connect(m_manager,SIGNAL(finished(QNetworkReply*)),SLOT(replyFinish(QNetworkReply*)));

    m_downTimer = new QTimer;
    m_downTimer->setSingleShot(true);
    connect(m_downTimer,SIGNAL(timeout()),SLOT(downloadTimeout()));

    connect(ui->btn_remotedownload,SIGNAL(clicked(bool)),SLOT(remoteDownload()));
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
    if(m_downTimer->isActive())
        m_downTimer->stop();
    m_downTimer->deleteLater();
}

void DebugDialog::show()
{
    QDialog::show();
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
    ui->cmb_resource->clear();
    for(int i = 0;i < resouces.size();++i)
    {
        ui->cmb_resource->insertItem(i,resouces[i]);
    }
}

void DebugDialog::setQXmppClient(QXmppClient *client)
{
    m_client = client;
}

void DebugDialog::messageReceived(const QXmppMessage &msg)
{
    QString showStr = QTime::currentTime().toString("[hh:mm:ss]");
    showStr += tr("[接收MESSAGE]:");
    showStr += msg.body();
    insertRecvMessageToTextBrowser(showStr);
    if(parseRemoteDownloadACK(showStr) == 1)
        m_downTimer->start(ATTIMEOUT);

    if(m_client)
        MessageLog::append(m_client->configuration().jidBare(),m_bareJid,showStr);//保存消息记录

}

void DebugDialog::presenceReceived(const QXmppPresence &presence)
{
    QString showStr = QTime::currentTime().toString("[hh:mm:ss]");
    showStr += tr("[接收PRESENCE]:");
    showStr += presence.statusText();

    insertPresenceToTextBrowser(showStr);//界面显示

    if(m_client)
        MessageLog::append(m_client->configuration().jidBare(),m_bareJid,showStr);//保存消息记录
}

void DebugDialog::sendMessage()
{
    if(m_client)
    {
        m_client->sendMessage(m_bareJid,ui->txb_send->toPlainText());
    }

    QString showStr = QTime::currentTime().toString("[hh:mm:ss]");
    showStr += tr("[发送MESSAGE]:");
    showStr +=  ui->txb_send->toPlainText();

    insertSendMessageToTextBrowser(showStr);//界面显示

    ui->txb_send->clear();

    if(m_client)
        MessageLog::append(m_client->configuration().jidBare(),m_bareJid,showStr);//保存消息记录
}

void DebugDialog::remoteDownload()
{

    QString resource = getDownloadResource();
    if(resource.isEmpty())
    {
        insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:resource is empty"));
        return;
    }
    insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:resource:%1").arg(resource));

    QString fileName = getDownloadFileName();
    if(fileName.isEmpty())
    {
        insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:filename is empty"));
        return;
    }
    insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:filename:%1").arg(fileName));

    m_downloadFile = new QFile(fileName);
    if(!m_downloadFile->open(QIODevice::ReadOnly))
    {
        insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:file open failed"));
        return;
    }
    ui->btn_remotedownload->setEnabled(false);
    m_fileMD5 = QCryptographicHash::hash(m_downloadFile->readAll(),QCryptographicHash::Md5).toHex().constData();//计算文件MD5
    insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:file md5:%1").arg(m_fileMD5));

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

    check = connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(replyError(QNetworkReply::NetworkError)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(uploadProgress(qint64,qint64)),SLOT(uploadProgress(qint64, qint64)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(finished()),SLOT(uploadFinish()));
    Q_ASSERT(check);

    m_lastDownloadAction = UPLOAD_TO_HTTPSERVER;
    m_downTimer->start(10000);
}

void DebugDialog::replyError(QNetworkReply::NetworkError error)
{
    insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:upload to server error:%1").arg(error));
    ui->btn_remotedownload->setEnabled(true);
    ui->pgb_download->hide();
}

void DebugDialog::uploadProgress(qint64 sendSize, qint64 totalSize)
{
    m_downTimer->start(HTTPTIMEROUT);
    if(totalSize > 0)
    {
        ui->pgb_download->show();
        int percentage = sendSize*100/totalSize;
        ui->pgb_download->setValue(percentage);
        insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:upload to server:%1").arg(percentage));
    }
}

void DebugDialog::uploadFinish()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    QString result(arr);
    insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:upload to server:%1").arg(QString(arr)));

    reply->deleteLater();
    m_downloadFile->close();
    m_downloadFile->deleteLater();
    m_downloadFile = 0;
    m_multiPart->deleteLater();
    m_multiPart = 0;
    if(!result.contains("./upload/"))//上传失败
    {
        insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:upload to server failed"));
        ui->btn_remotedownload->setEnabled(true);
        ui->pgb_download->hide();
        return;
    }
    QString atString = tr("at+down=http://115.28.44.147/upload2/upload/%1").arg(result.split('/').last());
    m_client->sendMessage(m_bareJid,atString);
    insertSendMessageToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[发送MESSAGE]:%1").arg(atString));
    m_lastDownloadAction = AT_DOWN;
    m_downTimer->start(HTTPTIMEROUT);
}

void DebugDialog::downloadTimeout()
{
    switch(m_lastDownloadAction)
    {
        case UPLOAD_TO_HTTPSERVER:
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:upload timeout"));
            break;
        case AT_DOWN:
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:AT DOWN timeout"));
            break;
        case AT_ISP:
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:AT ISP timeout"));
            break;
        case AT_RST:
            insertParseTextToTextBrowser(QTime::currentTime().toString("[hh:mm:ss]") + tr("[REMOTEDOWNLOAD]:AT RST timeout"));
            break;
        default:
            break;
    }
    ui->pgb_download->setValue(0);
    ui->pgb_download->hide();
    ui->btn_remotedownload->setEnabled(true);
}

void DebugDialog::keyPressEvent(QKeyEvent* event)
{
    QDialog::keyPressEvent(event);
    ui->txb_send->setFocus();
}
