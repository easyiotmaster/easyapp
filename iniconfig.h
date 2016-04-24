#ifndef INICONFIG_H
#define INICONFIG_H
#include <QSettings>
#include <QDir>

class IniConfig
{
private:
    QSettings*   configSettings;
public:
    explicit IniConfig();

    void loadConfigFromJid(const QString &jidBare);
    quint16 getTcpServerPort();
    bool    setTcpServerPort(quint16 port);
};

#endif // INICONFIG_H
