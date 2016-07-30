#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QSqlDatabase>
class SqlHelper
{
private:
    static QSqlDatabase*   m_mySqlDataBase;

public:
    static void initMySqlDataBase();
    static void deleteMySqlDataBase();
    static QString queryMySqlDatabase(const QString &query);//查询MYSQL数据库
};

#endif // SQLHELPER_H
