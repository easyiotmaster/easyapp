#include "sqlhelper.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QDebug>
QSqlDatabase*   SqlHelper::m_mySqlDataBase;
void SqlHelper::initMySqlDataBase()
{
    m_mySqlDataBase = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL","mysql"));
    m_mySqlDataBase->setHostName("127.0.0.1");
    m_mySqlDataBase->setPort(3306);
    m_mySqlDataBase->setDatabaseName("EASYIOT");
    m_mySqlDataBase->setUserName("root");
    //m_mySqlDataBase->setPassword("root");
}

void SqlHelper::deleteMySqlDataBase()
{
    if(m_mySqlDataBase->isOpen())
        m_mySqlDataBase->close();
    delete m_mySqlDataBase;
}

QString SqlHelper::queryMySqlDatabase(const QString &query)
{
    if(!m_mySqlDataBase->isOpen())
    {
        if(!m_mySqlDataBase->open())
        {
            return QObject::tr("MySQL数据库连接失败：%1").arg(m_mySqlDataBase->lastError().text());
        }
    }
    QSqlQuery sqlQuery = m_mySqlDataBase->exec(query);
    if(sqlQuery.lastError().isValid())
        return QObject::tr("MySQL数据库查询失败:%1").arg(sqlQuery.lastError().text());
    else
        return QObject::tr("MySQL数据库查询成功");

}
