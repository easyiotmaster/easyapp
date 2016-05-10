#include "otasetdialog.h"
#include "ui_otasetdialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QDataStream>
#include <QBuffer>
#include <QNetworkRequest>
#include <QStringList>
#include "QCryptographicHash"
#include <QHttpPart>
#include "mainDialog.h"
#include "iniconfig.h"
OTASetDialog::OTASetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OTASetDialog),m_client(0),m_barejid("")
  ,m_currentResource(""),m_fileName(""),m_fileMD5(""),m_file(0),m_multiPart(0),m_downTimer(0)
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("OTA-%1").arg(m_barejid));
    ui->cmb_selectsource->setCurrentText(m_currentResource);

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager,SIGNAL(finished(QNetworkReply*)),SLOT(replyFinish(QNetworkReply*)));

    m_downTimer = new QTimer;
    m_downTimer->setSingleShot(true);
    bool check;

    check = connect(ui->btn_selectfw,SIGNAL(clicked(bool)),SLOT(selectFWFile()));
    Q_ASSERT(check);

    check = connect(ui->btn_download,SIGNAL(clicked(bool)),SLOT(remoteDownload()));
    Q_ASSERT(check);

    check = connect(ui->cmb_selectsource,SIGNAL(activated(int)),SLOT(currentSourceChange(int)));
    Q_ASSERT(check);

    check = connect(m_downTimer,SIGNAL(timeout()),SLOT(downloadAckTimeout()));
    Q_ASSERT(check);
}

OTASetDialog::~OTASetDialog()
{
    delete ui;
}

void OTASetDialog::setQXmppClient(QXmppClient *client)
{
    m_client = client;
}

void OTASetDialog::setBareJid(const QString &bareJid)
{
    m_barejid = bareJid;
    setWindowTitle(QObject::tr("OTA-%1").arg(m_barejid));
}

void OTASetDialog::setResource(const QStringList &resouces)
{
    m_resourceList = resouces;
    ui->cmb_selectsource->clear();
    for(int i = 0;i < m_resourceList.size();++i)
    {
        ui->cmb_selectsource->addItem(m_resourceList[i]);
    }
    ui->cmb_selectsource->setCurrentText(m_currentResource);
}

void OTASetDialog::lockUI(bool lock)
{
    lock = !lock;
    ui->btn_selectfw->setEnabled(lock);
    ui->cmb_selectsource->setEnabled(lock);
    ui->btn_download->setEnabled(lock);
}

bool OTASetDialog::isResourceExist(const QString &resource)
{
    for(int i = 0;i < m_resourceList.size();++i)
    {
        if(resource == m_resourceList[i])
            return true;
    }
    return false;
}

void OTASetDialog::selectFWFile()
{
    m_fileName = QFileDialog::getOpenFileName(this,QObject::tr("选择固件"),IniConfig::getRemoteDownloadPath(m_client->configuration().jidBare()),tr("BIN(*.bin)"));
    ui->ldt_fwfilename->setText(m_fileName);
    IniConfig::setRemoteDownloadPath(m_fileName,m_client->configuration().jidBare());
}

void OTASetDialog::remoteDownload()
{
    lockUI(true);
    if(m_currentResource.isEmpty())
    {
        QMessageBox::about(this,tr("OTA"),tr("resource不能为空！"));
        lockUI(false);
        return;
    }
    if(!isResourceExist(m_currentResource))
    {
        QMessageBox::about(this,tr("OTA"),tr("resource不正确！"));
        lockUI(false);
        return;
    }
    if(m_fileName.isEmpty())
    {
        QMessageBox::about(this,"OTA",QObject::tr("请先选择需要下载的固件!"));
        lockUI(false);
        return;
    }
    if(m_file)
    {
        if(m_file->isOpen())
            m_file->close();
        m_file->deleteLater();
    }
    m_file = new QFile(m_fileName);
    if(!m_file->open(QIODevice::ReadOnly))
    {
        QMessageBox::about(this,"OTA",QObject::tr("打开固件失败!"));
        lockUI(false);
        return;
    }
    m_fileMD5 = QCryptographicHash::hash(m_file->readAll(),QCryptographicHash::Md5).toHex().constData();
    ui->txd_downloadlog->append(tr("文件MD5：%1").arg(m_fileMD5));
    m_file->reset();

    QNetworkRequest request(QUrl("http://115.28.44.147/upload2/upload.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data;boundary=1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik9ol0p");

    if(m_multiPart)
        m_multiPart->deleteLater();
    m_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    m_multiPart->setBoundary(QByteArray("1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik9ol0p"));

    QHttpPart imagePart;
    imagePart.setRawHeader(QByteArray("Content-Type"), QByteArray("application/octet-stream"));
    imagePart.setRawHeader(QByteArray("Content-Disposition"), tr("form-data;name=\"pic\";filename=\"%1\"").arg(m_fileName.split('/').last()).toLatin1());
    imagePart.setBodyDevice(m_file);

    m_file->setParent(m_multiPart); // we cannot delete the file now, so delete it with the multiPart
    m_multiPart->append(imagePart);

    QNetworkReply* reply = m_manager->post(request,m_multiPart);

    bool check;

    check = connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(replyError(QNetworkReply::NetworkError)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(uploadProgress(qint64,qint64)),SLOT(uploadProgress(qint64, qint64)));
    Q_ASSERT(check);

    m_lastDownloadAction = UPLOAD_TO_HTTPSERVER;
    m_downTimer->start(10000);
    //m_file->close();
    //m_file->deleteLater();
    //multiPart->deleteLater();
}

void OTASetDialog::currentSourceChange(int index)
{
    m_currentResource = ui->cmb_selectsource->itemText(index);
    ui->lab_source->setText(QObject::tr("source:%1").arg(ui->cmb_selectsource->itemText(index)));
}

void OTASetDialog::replyFinish(QNetworkReply *reply)
{
    QByteArray arr = reply->readAll();
    qDebug()<<__func__<<arr;
    QString result(arr);

    ui->txd_downloadlog->append(result);


    reply->deleteLater();
    m_file->close();
    m_file->deleteLater();
    m_file = 0;
    m_multiPart->deleteLater();
    m_multiPart = 0;

    if(!result.contains("./upload/"))//上传失败
    {
        lockUI(false);
        return;
    }
    QString atString = tr("at+down=http://115.28.44.147/upload2/upload/%1").arg(result.split('/').last());
    m_client->sendMessage(m_barejid,atString);
    ui->txd_downloadlog->append(tr("[AT]%1").arg(atString));
    m_lastDownloadAction = AT_DOWN;
    m_downTimer->start(HTTPTIMEROUT);
}

void OTASetDialog::replyFinished()
{
    QNetworkReply* reply = (QNetworkReply*)sender();

    QByteArray arr = reply->readAll();

    qDebug()<<__func__<<arr;
}

void OTASetDialog::replyError(QNetworkReply::NetworkError error)
{
    qDebug()<<error;
}

void OTASetDialog::uploadProgress(qint64 sendSize, qint64 totalSize)
{
    m_downTimer->start(HTTPTIMEROUT);
    if(totalSize > 0)
    {
        int percentage = sendSize*100/totalSize;
        ui->pgb_download->setValue(percentage);
        ui->txd_downloadlog->append(QObject::tr("上传至服务器：%1%").arg(percentage));
    }
}

void OTASetDialog::downloadAckTimeout()
{
    switch(m_lastDownloadAction)
    {
        case UPLOAD_TO_HTTPSERVER:
            ui->txd_downloadlog->append(tr("服务器访问超时！"));
            break;
        case AT_DOWN:
            ui->txd_downloadlog->append(tr("AT DOWN 超时！"));
            break;
        case AT_ISP:
            ui->txd_downloadlog->append(tr("AT ISP 超时！"));
                break;
        case AT_RST:
            ui->txd_downloadlog->append(tr("AT RST 超时！"));
                break;
        default:
            break;
    }
    ui->pgb_download->setValue(0);
    lockUI(false);
}

int OTASetDialog::recvClientMessage(const QString &msg)
{
    m_downTimer->start(ATTIMEOUT);
    ui->txd_downloadlog->append(tr("[ACK]%1").arg(msg));
    if(msg == "ERROR")
    {
        switch(m_lastDownloadAction)
        {
            case AT_DOWN:
                ui->txd_downloadlog->append(tr("AT DOWN ERROR"));
                break;
            case AT_ISP:
                ui->txd_downloadlog->append(tr("AT ISP ERROR"));
                    break;
            case AT_RST:
                ui->txd_downloadlog->append(tr("AT RST ERROR"));
                    break;
            default:
                return 0;
        }
        lockUI(false);
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
                ui->txd_downloadlog->append(tr("重启成功！"));
                    break;
            default:
                return 0;
        }

    }
    else if(msg.contains("+DOWN=ERROR"))
    {
        lockUI(false);
    }
    else if(msg.contains("+DOWN=MD5"))
    {
        ui->pgb_download->setValue(100);
        QStringList md5List = msg.split(',');
        QString md5;
        if(md5List.size() != 2)
        {
            ui->txd_downloadlog->append(tr("MD5解析错误！"));
            lockUI(false);
        }
        else
        {
            md5 = md5List[1];
            if(md5 != m_fileMD5)
            {
                ui->txd_downloadlog->append(tr("MD5不匹配！"));
                lockUI(false);
            }
            else
            {
                QString atString = tr("at+isp");
                m_client->sendMessage(m_barejid,atString);
                ui->txd_downloadlog->append(tr("[AT]%1").arg(atString));
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
            ui->txd_downloadlog->append(tr("DOWN下载长度解析失败！"));
            lockUI(false);
            return 1;
        }
        else
        {
            strList = strList[1].split(',');
            if(strList.size() != 2)
            {
                ui->txd_downloadlog->append(tr("DOWN下载长度解析失败！"));
                lockUI(false);
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
        ui->txd_downloadlog->append(tr("远程下载完成！"));
        /*QString atString = tr("at+rst");
        m_client->sendMessage(m_barejid,atString);
        ui->txd_downloadlog->append(tr("[AT]%1").arg(atString));
        m_lastDownloadAction = AT_RST;*/
        lockUI(false);
        m_downTimer->stop();
    }
    else if(msg.contains("+ISP=ERROR"))
    {
        lockUI(false);
    }
    else if(msg.contains("+ISP="))
    {
        QStringList strList = msg.split('=');
        QString downSizeStr,totalSizeStr;
        if(strList.size() != 2)
        {
            ui->txd_downloadlog->append(tr("ISP下载长度解析失败！"));
            lockUI(false);
            return 1;
        }
        else
        {
            strList = strList[1].split(',');
            if(strList.size() != 2)
            {
                ui->txd_downloadlog->append(tr("ISP下载长度解析失败！"));
                lockUI(false);
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
