#ifndef INICONFIG_H
#define INICONFIG_H
#include <QSettings>
#include <QDir>
#include <QHostAddress>
enum SQL_TYPE
{
    SQL_MYSQL = 0,
    SQL_SQLITE,
    SQL_ODBC
};

struct ConfigData{
    int     tcpServerPort;
    QString remoteDownloadPath;
    QString ledFilePath;
    QString otaFilePath;
    QString imageFilePath;
    SQL_TYPE sqlType;
    QHostAddress sqlHostAddress;
    QString sqlUserName;
    QString sqlPassword;
    int     sqlPort;
    QString sqlDatabase;
    QString firmwareServer;
    QString ledTemplateServer;

};

class IniConfig
{
private:
    //QSettings*   configSettings;
public:
    static ConfigData configData;

    explicit IniConfig();

    //void loadConfigFromJid(const QString &jidBare);
    static quint16 getTcpServerPort(const QString &jidBare);
    static void    setTcpServerPort(quint16 port,const QString &jidBare);

    static QString getConfigFileName(const QString &jidBare);
    static QString getRemoteDownloadPath(const QString &jidBare);
    static QString getLedFilePath(const QString &jidBare);
    static QString getOtaFilePath(const QString &jidBare);
    static QString getLedTemplateFilePath(const QString &jidBare);
    static QString getSaveImageFilePath(const QString &local_jidBare,const QString &remote_jidBare);

    static SQL_TYPE getSqlType(const QString &jidBare);
    static QString getSqlHost(const QString &jidBare);
    static QString getSqlUserName(const QString &jidBare);
    static QString getSqlPassword(const QString &jidBare);
    static int     getSqlPort(const QString &jidBare);
    static QString getSqlDatabase(const QString &jidBare);

    static QString getFirmwareServer(const QString &jidBare);
    static QString getLedTemplateServer(const QString &jidBare);
    static uint  getQuerySigPeriod(const QString &jidBare);
    static void    setRemoteDownloadPath(const QString &path,const QString &jidBare);
    static void    setLedFilePath(const QString &path,const QString &jidBare);
    static void    setOtaFilePath(const QString &path,const QString &jidBare);

    static void    setSqlType(const QString &jidBare,SQL_TYPE sqlType);
    static void    setSqlHost(const QString &jidBare,const QString &sqlHost);
    static void    setSqlUserName(const QString &jidBare,const QString &sqlUserName);
    static void    setSqlPassword(const QString &jidBare,const QString &sqlPassword);
    static void    setSqlPort(const QString &jidBare,const QString &sqlPort);
    static void    setSqlDatabase(const QString &jidBare,const QString &sqlDatabases);

    static void    setFirmwareServer(const QString &jidBare,const QString &firmwareServer);
    static void    setLedTemplateServer(const QString &jidBare,const QString &ledtemplateServer);

    static void    setQuerySigPeriod(const QString &jidBare,uint period);
};

#endif // INICONFIG_H
