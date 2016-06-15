#ifndef INICONFIG_H
#define INICONFIG_H
#include <QSettings>
#include <QDir>

class IniConfig
{
private:
    //QSettings*   configSettings;
public:
    explicit IniConfig();

    //void loadConfigFromJid(const QString &jidBare);
    static quint16 getTcpServerPort(const QString &jidBare);
    static void    setTcpServerPort(quint16 port,const QString &jidBare);

    static QString getConfigFileName(const QString &jidBare);
    static QString getRemoteDownloadPath(const QString &jidBare);
    static QString getLedFilePath(const QString &jidBare);
    static QString getLedTemplateFilePath(const QString &jidBare);
    static void    setRemoteDownloadPath(const QString &path,const QString &jidBare);
    static void    setLedFilePath(const QString &path,const QString &jidBare);
};

#endif // INICONFIG_H
