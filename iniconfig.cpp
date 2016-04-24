#include "iniconfig.h"
#include "utils.h"
#include "tcpserver.h"
IniConfig::IniConfig():configSettings(0)
{

}

void IniConfig::loadConfigFromJid(const QString &jidBare)
{
    QString path = getSettingsDir(jidBare);
    QDir dir;
    if(!dir.exists(path))
        dir.mkpath(path);

    QString configFileName = path + "config.ini";

    if(configSettings)
        delete configSettings;
    configSettings = new QSettings(configFileName,QSettings::IniFormat);
}

bool IniConfig::setTcpServerPort(quint16 port)
{
    if(!configSettings)
        return false;
    configSettings->setValue("tcp_server/port",port);
    configSettings->sync();
    return true;
}

quint16 IniConfig::getTcpServerPort()
{
    if(!configSettings)
        return DEFAULT_SERVER_PORT;
    quint16 port = configSettings->value("tcp_server/port",DEFAULT_SERVER_PORT).toUInt();
    configSettings->setValue("tcp_server/port",port);
    configSettings->sync();
    return port;
}

