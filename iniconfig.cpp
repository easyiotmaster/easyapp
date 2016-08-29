#include "iniconfig.h"
#include "utils.h"
#include "tcpserver.h"
ConfigData IniConfig::configData;
IniConfig::IniConfig()
{
}

QString IniConfig::getConfigFileName(const QString &jidBare)
{
    QString configPath = getSettingsDir(jidBare);
    QDir dir;
    if(!dir.exists(configPath))
        dir.mkpath(configPath);

    return configPath + "config.ini";
}

quint16 IniConfig::getTcpServerPort(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    quint16 port = settings.value("tcp_server/port",DEFAULT_SERVER_PORT).toUInt();
    settings.setValue("tcp_server/port",port);
    settings.sync();
    return port;
}

void IniConfig::setTcpServerPort(quint16 port, const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);

    settings.setValue("tcp_server/port",port);
    settings.sync();
}

QString IniConfig::getRemoteDownloadPath(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);

#if defined(WIN32)
    QString path = settings.value("remote_download/bin_path","C:/Users/Administrator/Desktop").toString();
#elif defined(LINUX)
    QString path = settings.value("remote_download/bin_path","/home").toString();
#else
    QString path = settings.value("remote_download/bin_path","").toString();
#endif
    settings.setValue("remote_download/bin_path",path);
    settings.sync();
    return path;
}

QString IniConfig::getLedFilePath(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);

#if defined(WIN32)
    QString path = settings.value("remote_download/led_bin_path","C:/Users/Administrator/Desktop").toString();
#elif defined(LINUX)
    QString path = settings.value("remote_download/led_bin_path","/home").toString();
#else
    QString path = settings.value("remote_download/led_bin_path","").toString();
#endif
    settings.setValue("remote_download/led_bin_path",path);
    settings.sync();
    return path;
}

QString IniConfig::getOtaFilePath(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);

#if defined(WIN32)
    QString path = settings.value("remote_download/ota_file_path","C:/Users/Administrator/Desktop").toString();
#elif defined(LINUX)
    QString path = settings.value("remote_download/ota_file_path","/home").toString();
#else
    QString path = settings.value("remote_download/ota_file_path","").toString();
#endif
    settings.setValue("remote_download/ota_file_path",path);
    settings.sync();
    return path;
}

QString IniConfig::getLedTemplateFilePath(const QString &jidBare)
{
    return getSettingsDir(jidBare);
}

QString IniConfig::getSaveImageFilePath(const QString &local_jidBare,const QString &remote_jidBare)
{
    QString imagePath = getSettingsDir(local_jidBare)+"images/"+remote_jidBare+"/";
    QDir dir;
    if(!dir.exists(imagePath))
        dir.mkpath(imagePath);

    return imagePath;
}

SQL_TYPE IniConfig::getSqlType(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString sqlType = settings.value("sql_config/sql_type").toString();
    if(sqlType == "MYSQL")
        return SQL_MYSQL;
    else if(sqlType == "SQLITE")
        return SQL_SQLITE;
    else if(sqlType == "ODBC")
        return SQL_ODBC;
    else
        return SQL_MYSQL;
}

QString IniConfig::getSqlHost(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString sqlHost = settings.value("sql_config/sql_host").toString();

    return sqlHost;
}

QString IniConfig::getSqlUserName(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString sqlUserName = settings.value("sql_config/sql_username").toString();
    return sqlUserName;
}

QString IniConfig::getSqlPassword(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString sqlPassword = settings.value("sql_config/sql_password").toString();
    return sqlPassword;
}

int IniConfig::getSqlPort(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    int sqlPort = settings.value("sql_config/sql_port").toInt();
    return sqlPort;
}

QString IniConfig::getSqlDatabase(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString sqlDatabase = settings.value("sql_config/sql_database").toString();
    return sqlDatabase;
}

QString IniConfig::getFirmwareServer(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString server_firmware = settings.value("server_config/server_firmware").toString();
    if(server_firmware == "")
    {
        setFirmwareServer(jidBare,server_firmware);
        return "http://115.28.44.147/upload2/upload.php";
    }
    return server_firmware;
}

QString IniConfig::getLedTemplateServer(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString server_ledtemplate = settings.value("server_config/server_ledtemplate").toString();
    if(server_ledtemplate == "")
    {
        setLedTemplateServer(jidBare,server_ledtemplate);
        return "http://www.easy-iot.cc/download/easyled_firmware.bin";
    }
    return server_ledtemplate;
}

uint IniConfig::getQuerySigPeriod(const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);
    bool ok;
    QSettings settings(configFileName,QSettings::IniFormat);
    uint query_sig_perid = settings.value("set/query_sig_period").toUInt(&ok);
    if(!ok)
    {
        query_sig_perid = 60;
        setQuerySigPeriod(jidBare,query_sig_perid);
    }
    return query_sig_perid;
}

void IniConfig::setRemoteDownloadPath(const QString &path, const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("remote_download/bin_path",path);
    settings.sync();
}

void IniConfig::setLedFilePath(const QString &path, const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("remote_download/led_bin_path",path);
    settings.sync();
}

void IniConfig::setOtaFilePath(const QString &path, const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("remote_download/ota_file_path",path);
    settings.sync();
}

void IniConfig::setSqlType(const QString &jidBare, SQL_TYPE sqlType)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    QString sql = "";
    if(sqlType == SQL_MYSQL)
        sql = "MYSQL";
    else if(sqlType == SQL_SQLITE)
        sql = "SQLITE";
    else if(sqlType == SQL_ODBC)
        sql = "ODBC";
    else
        sql = "MYSQL";
    settings.setValue("sql_config/sql_type",sql);
    settings.sync();
}

void IniConfig::setSqlHost(const QString &jidBare, const QString &sqlHost)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("sql_config/sql_host",sqlHost);
    settings.sync();
}

void IniConfig::setSqlUserName(const QString &jidBare, const QString &sqlUserName)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("sql_config/sql_username",sqlUserName);
    settings.sync();
}

void IniConfig::setSqlPassword(const QString &jidBare, const QString &sqlPassword)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("sql_config/sql_password",sqlPassword);
    settings.sync();
}

void IniConfig::setSqlPort(const QString &jidBare, const QString &sqlPort)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("sql_config/sql_port",sqlPort);
    settings.sync();
}

void IniConfig::setSqlDatabase(const QString &jidBare, const QString &sqlDatabases)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("sql_config/sql_database",sqlDatabases);
    settings.sync();
}

void IniConfig::setFirmwareServer(const QString &jidBare, const QString &firmwareServer)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("server_config/server_firmware",firmwareServer);
    settings.sync();
}

void IniConfig::setLedTemplateServer(const QString &jidBare, const QString &ledtemplateServer)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("server_config/server_ledtemplate",ledtemplateServer);
    settings.sync();
}

void IniConfig::setQuerySigPeriod(const QString &jidBare, uint period)
{
    QString configFileName = getConfigFileName(jidBare);
    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("set/query_sig_period",period);
    settings.sync();
}
