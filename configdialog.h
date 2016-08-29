#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QMap>
#include <QXmppClient.h>
#include "iniconfig.h"
namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
//    enum SQL_TYPE
//    {
//        SQL_MYSQL = 0,
//        SQL_SQLITE,
//        SQL_ODBC
//    };
    QXmppClient*    m_client;

    explicit ConfigDialog(QXmppClient *client, QWidget *parent = 0);
    ~ConfigDialog();

    void initConfigUIText();
    void initSqlTypeCombox();
    SQL_TYPE getSqlTypeFromText(const QString &sql);
    bool verificationInputText();
private:
    Ui::ConfigDialog *ui;

private slots:
    void testSqlConnection();
    bool applyModify();
    void confirmModify();
    void cancelModify();
};

#endif // CONFIGDIALOG_H
