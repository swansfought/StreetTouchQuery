#ifndef CONFIG_H
#define CONFIG_H

#include <QMutex>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QDir>

#define CURR_PATH QCoreApplication::applicationDirPath()
#define CFG_FILE_PATH CURR_PATH + "/cfg.json"

class Config
{
public:
    static Config* getInstance();

    void checkConfigFile(bool isCheck = true);

    bool updateRollStopTime(const int& sec);
    bool updateRollSpeed(const int& msec);
    bool updateSysIcon(const QByteArray& bytes);
    bool updateSysTitle(const QString& title);
    bool updateLocalTimeShow(const bool& bol);
    bool updateEditExitSave(const bool& bol);
    bool updateCloseWinTip(const bool& bol);
    bool updateRollRuleSet(const bool &bol);
    bool updateRemainLoginPwd(const bool &bol);
    bool updatePassword(const QString &password);
    bool updateStyle(const int &mode);

    int getRollStopTime() const;
    int getRollSpeed() const;
    QByteArray getSysIcon() const;
    QString getSysTitle() const;
    bool getLocalTimeShow() const;
    bool getEditExitSave() const;
    bool getCloseWinTip() const;
    bool getRollRuleSet() const;
    bool getRemainLoginPwd() const;
    QString getEncryPwd();
    int getStyleMode() const;

private:
    QJsonObject readCfgFlie();
    bool writeCfgFile(const QJsonObject &rootObj);
    QJsonObject getdefaultCfg();
    void loadCfg();
    QString encryptPassword(const QString& password); //加解密

    int rollStopTime;
    int rollSpeed;
    QByteArray sysIcon;
    QString sysTitle;
    bool localTimeShow;
    bool editExitSave;
    bool closeWinTip;
    bool rollRuleSet;
    bool remainLoginPwd;
    QString encryPwd;
    int styleMode;

private:
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static QMutex   mutex;
    static Config* instance;

};

#endif // CONFIG_H
