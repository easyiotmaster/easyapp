#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QSqlDatabase>
class SqlHelper
{
private:
    static QSqlDatabase*   m_mySqlDataBase;

public:
    static void initMySqlDataBase(const QString &jidBare);
    static void deleteMySqlDataBase();
    static QString queryMySqlDatabase(const QString &query,bool &ok);//查询MYSQL数据库
    static QString testSqlConnection(const QString &sqlType,const QString &sqlHost,const QString &sqlUserName,const QString &sqlPassword,const QString &port,const QString &sqlDatabase);
};

#endif // SQLHELPER_H
