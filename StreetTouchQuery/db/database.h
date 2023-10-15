#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QMessageBox>
#include <QFile>
#include <QCoreApplication>

#include <QDebug>
#include <db/record.h>

class DataBase
{

public:
    static DataBase* getInstance();
    bool connectDB();
    Record queryRecord(const int &buttonId);
    QVector<Record> queryAllRecord();

//    QString selectTitle(const int &btnId);
//    QString selectSubTitle(const int &btnId);
//    QString selectContext(const int &btnId);
//    int selectShowMode(const int &btnId);
//    QByteArray selectImage(const int &btnId);

    bool updateTitle(const int &btnId, const QString &title);
    bool updateSubTitle(const int &btnId, const QString &subTitle);
    bool updateContext(const int &btnId, const QString &context);
    bool updateShowMode(const int &btnId, const int &mode);
    bool updateImage(const int &btnId, const QByteArray &bytes);

    bool deleteImage(const int &btnId);

private:
    DataBase();
    ~DataBase();
    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    static QMutex mutex;
    static DataBase* instance;


    bool connectState;
    QSqlDatabase db;

};

#endif // DATABASE_H
