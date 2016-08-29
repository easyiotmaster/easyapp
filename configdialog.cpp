#include "configdialog.h"
#include "ui_configdialog.h"
#include "sqlhelper.h"
#include <QMessageBox>
ConfigDialog::ConfigDialog(QXmppClient *client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    m_client = client;

    initSqlTypeCombox();

    connect(ui->btn_sqlconntest,SIGNAL(clicked(bool)),SLOT(testSqlConnection()));
    connect(ui->btn_config_apply,SIGNAL(clicked(bool)),SLOT(applyModify()));
    connect(ui->btn_config_ok,SIGNAL(clicked(bool)),SLOT(confirmModify()));
    connect(ui->btn_config_cancel,SIGNAL(clicked(bool)),SLOT(cancelModify()));

}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::initConfigUIText()
{
    QString jidBare = m_client->configuration().jidBare();
    ui->cmb_sql_type->setCurrentIndex(IniConfig::getSqlType(jidBare));
    ui->ldt_sql_host->setText(IniConfig::getSqlHost(jidBare));
    ui->ldt_sql_username->setText(IniConfig::getSqlUserName(jidBare));
    ui->ldt_sql_password->setText(IniConfig::getSqlPassword(jidBare));
    if(IniConfig::getSqlPort(jidBare) <= 0)
        ui->ldt_sql_port->setText("");
    else
        ui->ldt_sql_port->setText(QString::number(IniConfig::getSqlPort(jidBare)));
    ui->ldt_sql_database->setText(IniConfig::getSqlDatabase(jidBare));
    ui->ldt_firmware_addr->setText(IniConfig::getFirmwareServer(jidBare));
    ui->ldt_led_template_addr->setText(IniConfig::getLedTemplateServer(jidBare));

    ui->ldt_sig_query_period->setText(QString::number(IniConfig::getQuerySigPeriod(jidBare)));
}

void ConfigDialog::initSqlTypeCombox()
{
    ui->cmb_sql_type->addItem("MYSQL");
    ui->cmb_sql_type->addItem("SQLITE");
    ui->cmb_sql_type->addItem("ODBC");
}

SQL_TYPE ConfigDialog::getSqlTypeFromText(const QString &sql)
{
    if(sql == "MYSQL")
        return SQL_MYSQL;
    else if(sql == "SQLITE")
        return SQL_SQLITE;
    else if(sql == "ODBC")
        return SQL_ODBC;
    else
        return SQL_MYSQL;
}

bool ConfigDialog::verificationInputText()
{
    bool ok;
    QRegExp rx_ip("(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])");
    bool match = rx_ip.exactMatch(ui->ldt_sql_host->text());
    if(!match && ui->ldt_sql_host->text() != "localhost")
    {
        QMessageBox::about(this,QObject::tr("数据库"),tr("主机地址错误"));
        return false;
    }
    int port = ui->ldt_sql_port->text().toUInt(&ok);
    if(!ok || port == 0)
    {
        QMessageBox::about(this,QObject::tr("数据库"),tr("端口设置不正确"));
        return false;
    }
    if(ui->ldt_sql_username->text() == "")
    {
        QMessageBox::about(this,QObject::tr("数据库"),tr("用户名不能为空"));
        return false;
    }

    int period = ui->ldt_sig_query_period->text().toUInt(&ok);
    if(period == 0 || !ok)
    {
        QMessageBox::about(this,QObject::tr("其他"),tr("信号查询周期不正确"));
        return false;
    }
    return true;
}

void ConfigDialog::testSqlConnection()
{
    if(!verificationInputText())
        return;
    QString sqlType;
    if(ui->cmb_sql_type->currentIndex() == SQL_MYSQL)
        sqlType = "MYSQL";
    else if(ui->cmb_sql_type->currentIndex() == SQL_SQLITE)
        sqlType = "SQLITE";
    else if(ui->cmb_sql_type->currentIndex() == SQL_ODBC)
        sqlType = "ODBC";
    else
        sqlType = "MYSQL";
    QString tip = SqlHelper::testSqlConnection(sqlType,ui->ldt_sql_host->text(),ui->ldt_sql_username->text(),ui->ldt_sql_password->text(),ui->ldt_sql_port->text(),ui->ldt_sql_database->text());

    QMessageBox::about(this,QObject::tr("数据库连接信息"),tip);
}

bool ConfigDialog::applyModify()
{
    if(!verificationInputText())
        return false;
    QString jidBare = m_client->configuration().jidBare();

    IniConfig::setSqlType(jidBare,SQL_TYPE(ui->cmb_sql_type->currentIndex()));

    if(ui->ldt_sql_host->isModified())
        IniConfig::setSqlHost(jidBare,ui->ldt_sql_host->text());

    if(ui->ldt_sql_username->isModified())
        IniConfig::setSqlUserName(jidBare,ui->ldt_sql_username->text());

    if(ui->ldt_sql_password->isModified())
        IniConfig::setSqlPassword(jidBare,ui->ldt_sql_password->text());

    if(ui->ldt_sql_port->isModified())
        IniConfig::setSqlPort(jidBare,ui->ldt_sql_port->text());

    if(ui->ldt_sql_database->isModified())
        IniConfig::setSqlDatabase(jidBare,ui->ldt_sql_database->text());

    if(ui->ldt_firmware_addr->isModified())
        IniConfig::setFirmwareServer(jidBare,ui->ldt_firmware_addr->text());

    if(ui->ldt_led_template_addr->isModified())
        IniConfig::setLedTemplateServer(jidBare,ui->ldt_led_template_addr->text());

    SqlHelper::initMySqlDataBase(jidBare);

    if(ui->ldt_sig_query_period->isModified())
        IniConfig::setQuerySigPeriod(jidBare,ui->ldt_sig_query_period->text().toUInt());

    return true;
}

void ConfigDialog::confirmModify()
{
    if(applyModify())
        this->close();
}

void ConfigDialog::cancelModify()
{
    this->close();
}
