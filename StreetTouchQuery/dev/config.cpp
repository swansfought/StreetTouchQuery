#include "config.h"


QMutex Config::mutex;
Config* Config::instance=nullptr;

Config *Config::getInstance()
{
    if (instance == nullptr) {
        QMutexLocker locker(&mutex);
        if (instance == nullptr) {
            instance = new Config;
        }
    }
    return instance;
}

// true为检查，false为加载默认配置
void Config::checkConfigFile(bool isCheck)
{
    //1.配置文件是否存在
    QFile file(CFG_FILE_PATH);
    if(isCheck && file.exists()){
        loadCfg(); //加载配置
        return;
    }

    //2.新建配置文件
    QJsonDocument doc( getdefaultCfg() );
    QByteArray bytes = doc.toJson(QJsonDocument::Compact);

    if(file.open(QIODevice::ReadWrite)){ //用户配置文件
        file.write(bytes);
        file.close();
    }
    loadCfg(); //加载配置
}

bool Config::updateRollStopTime(const int &sec)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["roll_set"].toObject();
    subObj["roll_stop_time"] = sec;
    rootObj["roll_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        rollStopTime = sec;
        return true;
    }
    return false;
}

bool Config::updateRollSpeed(const int &msec)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["roll_set"].toObject();
    subObj["roll_speed"] = msec;
    rootObj["roll_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        rollSpeed = msec;
        return true;
    }
    return false;
}

bool Config::updateSysIcon(const QByteArray &bytes)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["icon_title_set"].toObject();
    subObj["icon"] = QJsonValue::fromVariant(bytes.toBase64());
    rootObj["icon_title_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        sysIcon = bytes;
        return true;
    }
    return false;
}

bool Config::updateSysTitle(const QString &title)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["icon_title_set"].toObject();
    subObj["title"] = title;
    rootObj["icon_title_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        sysTitle = title;
        return true;
    }
    return false;
}

bool Config::updateLocalTimeShow(const bool &bol)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["other_set"].toObject();
    subObj["local_time_show"] = bol;
    rootObj["other_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        localTimeShow = bol;
        return true;
    }
    return false;
}

bool Config::updateEditExitSave(const bool &bol)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["other_set"].toObject();
    subObj["edit_exit_save"] = bol;
    rootObj["other_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        editExitSave = bol;
        return true;
    }
    return false;
}

bool Config::updateCloseWinTip(const bool &bol)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["other_set"].toObject();
    subObj["close_win_tip"] = bol;
    rootObj["other_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        closeWinTip = bol;
        return true;
    }
    return false;
}

bool Config::updateRollRuleSet(const bool &bol)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["other_set"].toObject();
    subObj["roll_rule_set"] = bol;
    rootObj["other_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        rollRuleSet = bol;
        return true;
    }
    return false;
}

bool Config::updateRemainLoginPwd(const bool &bol)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["pwd_set"].toObject();
    subObj["remain_login_pwd"] = bol;
    rootObj["pwd_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        remainLoginPwd = bol;
        return true;
    }
    return false;
}

bool Config::updatePassword(const QString &password)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["pwd_set"].toObject();
    subObj["encry_pwd"] = encryptPassword(password);
    rootObj["pwd_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        encryPwd = encryptPassword(password);
        return true;
    }
    return false;
}

bool Config::updateStyle(const int &mode)
{
    //1.获取文件数据
    QJsonObject rootObj = readCfgFlie();
    if(rootObj.isEmpty())
        return false;

    //2.修改数据
    QJsonObject subObj = rootObj["style_set"].toObject();
    subObj["style_mode"] = mode;
    rootObj["style_set"] = subObj;

    //3.写回文件
    if(writeCfgFile(rootObj)){
        styleMode = mode;
        return true;
    }
    return false;
}

//写入配置数据
bool Config::writeCfgFile(const QJsonObject &rootObj)
{
    QFile file(CFG_FILE_PATH);
    if(file.open(QIODevice::WriteOnly)){
        QJsonDocument newDoc(rootObj);
        file.write(newDoc.toJson(QJsonDocument::Compact));
        file.close();
        return true;
    }
    return false;
}

QJsonObject Config::getdefaultCfg()
{
    QJsonObject rootObj;
    QJsonObject rollObj;
    rollObj.insert("roll_stop_time",2);
    rollObj.insert("roll_speed",30);

    QJsonObject sysObj;
    QFile imgFile(":/img/logo-red.png"); //默认图标
    QByteArray imgBase64;
    if(imgFile.open(QIODevice::ReadOnly))
        imgBase64 = imgFile.readAll().toBase64();
    sysObj.insert("icon",QJsonValue::fromVariant(imgBase64));
    sysObj.insert("title","蒋乔街道触摸查询");

    QJsonObject ohterObj;
    ohterObj.insert("local_time_show",false);
    ohterObj.insert("edit_exit_save",false);
    ohterObj.insert("close_win_tip",false);
    ohterObj.insert("roll_rule_set",false);

    QJsonObject pwdObj;
    pwdObj.insert("remain_login_pwd",false);
    pwdObj.insert("encry_pwd",encryptPassword("12345678"));

    QJsonObject styleObj;
    styleObj.insert("style_mode",2); //默认自然风景

    rootObj.insert("roll_set",rollObj);
    rootObj.insert("icon_title_set",sysObj);
    rootObj.insert("other_set",ohterObj);
    rootObj.insert("pwd_set",pwdObj);
    rootObj.insert("style_set",styleObj);

    return rootObj;
}

void Config::loadCfg()
{
    QJsonObject rootObj = readCfgFlie();
    QJsonObject sysObj = rootObj["icon_title_set"].toObject();
    QJsonObject otherObj = rootObj["other_set"].toObject();
    QJsonObject pwdObj = rootObj["pwd_set"].toObject();
    QJsonObject rollObj = rootObj["roll_set"].toObject();
    QJsonObject styleObj = rootObj["style_set"].toObject();
    rollStopTime = rollObj["roll_stop_time"].toInt();
    rollSpeed = rollObj["roll_speed"].toInt();
    //图标从base64解码
    QByteArray base64 = sysObj["icon"].toVariant().toByteArray();
    sysIcon = QByteArray::fromBase64(base64);
    sysTitle = sysObj["title"].toString();
    localTimeShow = otherObj["local_time_show"].toBool();
    editExitSave = otherObj["edit_exit_save"].toBool();
    closeWinTip = otherObj["close_win_tip"].toBool();
    rollRuleSet = otherObj["roll_rule_set"].toBool();
    remainLoginPwd = pwdObj["remain_login_pwd"].toBool();
    encryPwd = pwdObj["encry_pwd"].toString();
    styleMode = styleObj["style_mode"].toInt();
}

QString Config::encryptPassword(const QString &password)
{
    if(password.isEmpty())
        return "";

    static QByteArray key = "H&ide$Pas%swo^rd";
    QByteArray arrayPwd = password.toUtf8();
    int size = arrayPwd.size();
    //加密&解密
    for(int i=0; i<size; ++i)
        arrayPwd[i] = arrayPwd[i] ^ key[i % key.size()];
    return QString::fromUtf8(arrayPwd);
}

//获取配置文件数据
QJsonObject Config::readCfgFlie()
{
    QFile file(CFG_FILE_PATH);
    if(file.open(QIODevice::ReadOnly)){
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        return doc.object();
    }
    QJsonObject rootObj;
    return rootObj;
}

QString Config::getEncryPwd()
{
    return encryptPassword(encryPwd);
}

bool Config::getRemainLoginPwd() const
{
    return remainLoginPwd;
}

bool Config::getRollRuleSet() const
{
    return rollRuleSet;
}

bool Config::getCloseWinTip() const
{
    return closeWinTip;
}

bool Config::getEditExitSave() const
{
    return editExitSave;
}

bool Config::getLocalTimeShow() const
{
    return localTimeShow;
}

QString Config::getSysTitle() const
{
    return sysTitle;
}

QByteArray Config::getSysIcon() const
{
    return sysIcon;
}

int Config::getRollSpeed() const
{
    return rollSpeed;
}

int Config::getRollStopTime() const
{
    return rollStopTime;
}

int Config::getStyleMode() const
{
    return styleMode;
}

Config::Config()
{

}
