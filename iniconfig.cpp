#include "iniconfig.h"
#include "utils.h"
#include "tcpserver.h"
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

void IniConfig::setRemoteDownloadPath(const QString &path, const QString &jidBare)
{
    QString configFileName = getConfigFileName(jidBare);

    QSettings settings(configFileName,QSettings::IniFormat);
    settings.setValue("remote_download/bin_path",path);
    settings.sync();
}
