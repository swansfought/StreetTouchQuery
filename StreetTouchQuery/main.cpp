#include "mainwin.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    static QSharedMemory *singleApp = new QSharedMemory("SingleApp");//创建“SingleApp”的共享内存块
    if(!singleApp->create(1)){
        QMessageBox::information(NULL,"程序提示","已经有一个相同的程序在运行!");
        qApp->quit();
        return -1;
    }
    MainWin w;
    w.setWindowTitle(w.getSysName());
    w.show();
    return a.exec();
}
