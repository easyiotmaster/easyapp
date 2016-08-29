#include "sqlhelper.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QDebug>
#include"iniconfig.h"
QSqlDatabase*   SqlHelper::m_mySqlDataBase = 0;
void SqlHelper::initMySqlDataBase(const QString &jidBare)
{
    if(m_mySqlDataBase)
    {
        m_mySqlDataBase->close();
        delete m_mySqlDataBase;
    }
    SQL_TYPE sqlType = IniConfig::getSqlType(jidBare);
    if(sqlType == SQL_MYSQL)
        m_mySqlDataBase = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL","mysql"));
    else if(sqlType == SQL_SQLITE)
        m_mySqlDataBase = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE","sqlite"));
    else if(sqlType == SQL_ODBC)
        m_mySqlDataBase = new QSqlDatabase(QSqlDatabase::addDatabase("QODBC","odbc"));
    else
        m_mySqlDataBase = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL","mysql"));

    m_mySqlDataBase->setHostName(IniConfig::getSqlHost(jidBare));
    m_mySqlDataBase->setPort(IniConfig::getSqlPort(jidBare));
    m_mySqlDataBase->setDatabaseName(IniConfig::getSqlDatabase(jidBare));
    m_mySqlDataBase->setUserName(IniConfig::getSqlUserName(jidBare));
    m_mySqlDataBase->setPassword(IniConfig::getSqlPassword(jidBare));
}

void SqlHelper::deleteMySqlDataBase()
{
    if(m_mySqlDataBase->isOpen())
        m_mySqlDataBase->close();
    delete m_mySqlDataBase;
}

QString SqlHelper::queryMySqlDatabase(const QString &query, bool &ok)
{
    ok = false;
    if(m_mySqlDataBase == 0)
        return QObject::tr("数据库连接失败！");

    if(!m_mySqlDataBase->isOpen())
    {
        if(!m_mySqlDataBase->open())
        {
            return QObject::tr("数据库连接失败：%1").arg(m_mySqlDataBase->lastError().text());
        }
    }
    QSqlQuery sqlQuery = m_mySqlDataBase->exec(query);
    if(sqlQuery.lastError().isValid())
        return QObject::tr("数据库查询失败:%1").arg(sqlQuery.lastError().text());
    else
    {
        ok = true;
        return QObject::tr("数据库查询成功");
    }

}

QString SqlHelper::testSqlConnection(const QString &sqlType, const QString &sqlHost, const QString &sqlUserName, const QString &sqlPassword, const QString &port, const QString &sqlDatabase)
{
    QSqlDatabase* database;

    if(sqlType == "MYSQL")
        database = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL","mysql1"));
    else if(sqlType == "SQLITE")
        database = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE","sqlite1"));
    else if(sqlType == "ODBC")
        database = new QSqlDatabase(QSqlDatabase::addDatabase("QODBC","odbc1"));
    else
        database = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL","mysql1"));

    database->setHostName(sqlHost);
    database->setPort(port.toInt());
    database->setDatabaseName(sqlDatabase);
    database->setUserName(sqlUserName);
    database->setPassword(sqlPassword);

    QString str;
    if(!database->open())
    {

        str = QObject::tr("数据库连接失败：%1").arg(database->lastError().text());
    }
    else
    {
        str = QObject::tr("数据库连接成功！");
        database->close();
    }

    delete database;

    return str;

}
