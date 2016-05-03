#include "otasetdialog.h"
#include "ui_otasetdialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QDataStream>
#include <QBuffer>
#include <QNetworkRequest>
#include <QStringList>
OTASetDialog::OTASetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OTASetDialog),m_client(0),m_barejid("")
  ,m_currentResource("")
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("OTA-%1").arg(m_barejid));
    ui->cmb_selectsource->setCurrentText(m_currentResource);

    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),SLOT(replyFinish(QNetworkReply*)));

    bool check;

    check = connect(ui->btn_selectfw,SIGNAL(clicked(bool)),SLOT(selectFWFile()));
    Q_ASSERT(check);

    check = connect(ui->btn_download,SIGNAL(clicked(bool)),SLOT(remoteDownload()));
    Q_ASSERT(check);

    check = connect(ui->cmb_selectsource,SIGNAL(activated(int)),SLOT(currentSourceChange(int)));
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
        if(m_currentResource == m_resourceList[i])
            return true;
    }
    return false;
}

void OTASetDialog::selectFWFile()
{
    ui->ldt_fwfilename->setText(QFileDialog::getOpenFileName(this,QObject::tr("选择固件"),"",tr("PNG(*.png)")));
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
    QString fileName = ui->ldt_fwfilename->text();
    if(fileName.isEmpty())
    {
        QMessageBox::about(this,"OTA",QObject::tr("请先选择需要下载的固件!"));
        lockUI(false);
        return;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::about(this,"OTA",QObject::tr("打开固件失败!"));
        lockUI(false);
        return;
    }
    int len = file.size();
    QDataStream stream(&file);
    char* buff = new char[len];
    stream.readRawData(buff,len);
    file.close();

    QNetworkRequest request(QUrl("http://115.28.44.147/upload2/upload.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QByteArray dataArr(buff,len);
    QNetworkReply* reply = manager->post(request,dataArr);

    bool check;
    check = connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(replyError(QNetworkReply::NetworkError)));
    Q_ASSERT(check);

    check = connect(reply,SIGNAL(uploadProgress(qint64,qint64)),SLOT(uploadProgress(qint64, qint64)));
    Q_ASSERT(check);

    delete buff;
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
    if(result.contains("<title>") && result.contains("</title>"))
        ui->txd_downloadlog->append(result.split("<title>")[1].split("</title>")[0]);
    lockUI(false);
    reply->deleteLater();
}

void OTASetDialog::replyError(QNetworkReply::NetworkError error)
{
    qDebug()<<error;
}

void OTASetDialog::uploadProgress(qint64 sendSize, qint64 totalSize)
{
    if(totalSize > 0)
    {
        int percentage = sendSize*100/totalSize;
        ui->pgb_download->setValue(percentage);

        QTextCursor txtcur= ui->txd_downloadlog->textCursor();
        txtcur.movePosition(QTextCursor::End);
        txtcur.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
        QString lastLine = txtcur.selectedText();
        if(lastLine.contains(QObject::tr("上传至服务器：")))
            txtcur.removeSelectedText();
        ui->txd_downloadlog->append(QObject::tr("上传至服务器：%1%").arg(percentage));
    }
}
