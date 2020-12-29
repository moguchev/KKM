QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ComReader/ComChannel.cpp \
    ComReader/FSParser.cpp \
    ComReader/Hardware.cpp \
    ComReader/TLV.cpp \
    commandwindow.cpp \
    historydb.cpp \
    loginoperatorofd.cpp \
    main.cpp \
    mainwindow.cpp \
    productdb.cpp \
    registrationwindow.cpp \
    settingkkmwindow.cpp \
    settingprogwindow.cpp

HEADERS += \
    ComReader/CRC16.h \
    ComReader/ComChannel.h \
    ComReader/FSParser.h \
    ComReader/Hardware.h \
    ComReader/TLV.h \
    ComReader/utils.h \
    commandwindow.h \
    historydb.h \
    loginoperatorofd.h \
    mainwindow.h \
    productdb.h \
    registrationwindow.h \
    settingkkmwindow.h \
    settingprogwindow.h

FORMS += \
    commandwindow.ui \
    loginoperatorofd.ui \
    mainwindow.ui \
    registrationwindow.ui \
    settingkkmwindow.ui \
    settingprogwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
