QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

RC_ICONS = Applcon.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dev/config.cpp \
    db/database.cpp \
    db/record.cpp \
    main.cpp \
    mainwin.cpp \

HEADERS += \
    dev/config.h \
    db/database.h \
    db/record.h \
    mainwin.h \

FORMS += \
    mainwin.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    cmark/cmarkConfig.cmake.in \
    cmark/entities.inc \
    cmark/libcmark.pc.in
