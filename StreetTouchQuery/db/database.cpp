#include "database.h"

QMutex DataBase::mutex;
DataBase* DataBase::instance = nullptr;

DataBase *DataBase::getInstance()
{
    if(nullptr == instance){
        QMutexLocker locker(&mutex);
        if(nullptr == instance)
            instance = new DataBase;
    }
    return instance;
}

DataBase::DataBase():connectState(false)
{
}

bool DataBase::connectDB()
{
    if(connectState)
        return connectState;

    QString dbName = QCoreApplication::applicationDirPath() + "/data.db";
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName); //../DataBase/data.db
    QFile file(dbName);
    if(file.exists() && db.open())
        connectState = true;
    else
        connectState = false;

    return connectState;
}

Record DataBase::queryRecord(const int &buttonId)
{
    Record record;

    QString sql = QString("select * from datatable where buttonid=%1;").arg(buttonId);
    QSqlQuery query;
    if(query.exec(sql) && query.next()){
        record.setButtonid(buttonId);
        record.setTitle(query.value("title").toString());
        record.setSubtitle(query.value("subtitle").toString());
        record.setContext(query.value("context").toString());
        record.setShowmode(query.value("showmode").toInt());
        record.setImage(query.value("image").toByteArray());
        record.setUpdatetime(query.value("updatetime").toDateTime());
    }
    return record;
}

QVector<Record> DataBase::queryAllRecord()
{
    QVector<Record> vec;

    //为保险起见，工具按钮id依次查询数据
    Record record = queryRecord(1);
    vec.append(record);

    record = queryRecord(2);
    vec.append(record);

    record = queryRecord(21);
    vec.append(record);

    record = queryRecord(22);
    vec.append(record);

    record = queryRecord(3);
    vec.append(record);

    record = queryRecord(31);
    vec.append(record);

    record = queryRecord(32);
    vec.append(record);

    record = queryRecord(33);
    vec.append(record);

    record = queryRecord(34);
    vec.append(record);

    record = queryRecord(35);
    vec.append(record);

    record = queryRecord(36);
    vec.append(record);

    record = queryRecord(37);
    vec.append(record);

    record = queryRecord(38);
    vec.append(record);

    record = queryRecord(39);
    vec.append(record);

    record = queryRecord(4);
    vec.append(record);

    record = queryRecord(5);
    vec.append(record);

    record = queryRecord(6);
    vec.append(record);

    return vec;
}

bool DataBase::updateTitle(const int &btnId, const QString &title)
{
    QSqlQuery query;
    QString sql = QString("update datatable "
                          "set title='%1',updatetime='%2' "
                          "where buttonid=%3;").arg(title,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),QString::number(btnId));
    if(query.exec(sql))
        return true;
    return false;
}

bool DataBase::updateSubTitle(const int &btnId, const QString &subTitle)
{
    QSqlQuery query;
    QString sql = QString("update datatable "
                          "set subtitle='%1',updatetime='%2' "
                          "where buttonid=%3;").arg(subTitle,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),QString::number(btnId));
    if(query.exec(sql))
        return true;
    return false;
}

bool DataBase::updateContext(const int &btnId, const QString &context)
{
    QSqlQuery query;
    query.prepare("update datatable set context=?,updatetime=? where buttonid=?;");
    query.bindValue(0,context);
    query.bindValue(1,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(2,btnId);
    if(query.exec())
        return true;
    return false;
}

bool DataBase::updateShowMode(const int &btnId, const int &mode)
{
    if(0 != mode && 1 != mode)
        return false;
    QSqlQuery query;
    QString sql = QString("update datatable "
                          "set showmode=%1,updatetime='%2' "
                          "where buttonid=%3;").arg(QString::number(mode),QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),QString::number(btnId));
    if(query.exec(sql))
        return true;
    return false;
}

bool DataBase::updateImage(const int &btnId, const QByteArray &bytes)
{
    QSqlQuery query;
    query.prepare("update datatable set image=?,updatetime=? where buttonid=?;");
    query.bindValue(0,bytes);
    query.bindValue(1,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(2,btnId);
    if(query.exec())
        return true;
    return false;
}

bool DataBase::deleteImage(const int &btnId)
{
    QSqlQuery query;
    query.prepare("update datatable set image=null,showmode=0,updatetime=? where buttonid=?;");
    query.bindValue(0,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(1,btnId);
    if(query.exec())
        return true;
    return false;
}
