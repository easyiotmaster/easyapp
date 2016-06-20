#include "tcpsetdialog.h"
#include "ui_tcpsetdialog.h"
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include "mainDialog.h"
TcpSetDialog::TcpSetDialog(mainDialog *parent) :
    QDialog(parent),
    ui(new Ui::TcpSetDialog)
{
    ui->setupUi(this);
}

TcpSetDialog::~TcpSetDialog()
{
    delete ui;
}

void TcpSetDialog::setShowTcpServerPort(quint16 port)
{
    ui->lab_port->setText(tr("端口:%1").arg(port));
    ui->ldt_port->setText(tr("%1").arg(port));
}

void TcpSetDialog::setTcpServerStatus(bool accept)
{
    ui->btn_startaccept->setEnabled(!accept);
    ui->btn_stopaccept->setEnabled(accept);
}

void TcpSetDialog::on_btn_setport_clicked()
{
    bool ok;

    quint16 port = ui->ldt_port->text().toUInt(&ok);

    if(!ok || port == 0)
    {
        QMessageBox::about(this,tr("Tcp 设置"),tr("端口监听失败 !"));
        return;
    }

    ui->lab_port->setText(QObject::tr("端口;%1").arg(port));//界面更新

    qobject_cast<mainDialog*>(this->parent())->setTcpServerPort(port);//写入配置
    on_btn_stopaccept_clicked();
}

void TcpSetDialog::on_btn_startaccept_clicked()
{
    if(qobject_cast<mainDialog*>(this->parent())->startTcpServer())
    {
        ui->btn_startaccept->setEnabled(false);
        ui->btn_stopaccept->setEnabled(true);
    }
    else
        QMessageBox::about(this,tr("Tcp 设置"),QObject::tr("启动监听失败 !"));

}

void TcpSetDialog::on_btn_stopaccept_clicked()
{
    qobject_cast<mainDialog*>(this->parent())->stopTcpServer();
    ui->btn_startaccept->setEnabled(true);
    ui->btn_stopaccept->setEnabled(false);
}
