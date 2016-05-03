#ifndef TCPSETDIALOG_H
#define TCPSETDIALOG_H

#include <QDialog>
#include <tcpserver.h>

namespace Ui {
class TcpSetDialog;
}
class mainDialog;
class TcpSetDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TcpSetDialog(mainDialog *parent);
    ~TcpSetDialog();

    void setShowTcpServerPort(quint16 port);
    void setTcpServerStatus(bool accept);
signals:
    void setTcpServerPort(quint16 port);
    void startTcpServer();
    void stopTcpServer();

private slots:
    void on_btn_setport_clicked();
    void on_btn_startaccept_clicked();
    void on_btn_stopaccept_clicked();

private:
    Ui::TcpSetDialog *ui;
};

#endif // TCPSETDIALOG_H
