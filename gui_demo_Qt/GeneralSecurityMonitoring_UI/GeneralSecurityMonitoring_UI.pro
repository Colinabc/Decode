#-------------------------------------------------
#
# Project created by QtCreator 2020-06-04T11:01:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GeneralSecurityMonitoring_UI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
    src/cameradecode.cpp \
    src/videolabel.cpp \
    src/generalsecuritymonitor.cpp \
    src/publicheader.cpp \
    src/timerthread.cpp

HEADERS += \
        inc/mainwindow.h \
     inc/cameradecode.h \
     inc/videolabel.h \
    inc/publicheader.h \
    inc/generalsecuritymonitor.h \
    inc/timerthread.h

FORMS += \
        ui/mainwindow.ui

RESOURCES += \
    images.qrc

INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/inc

unix: CONFIG += link_pkgconfig c++11
unix: PKGCONFIG += opencv

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

unix:!macx: LIBS += -L/usr/local/lib/ -lavformat
unix:!macx: LIBS += -L/usr/local/lib/ -lavfilter
unix:!macx: LIBS += -L/usr/local/lib/ -lavdevice
unix:!macx: LIBS += -L/usr/local/lib/ -lswresample
unix:!macx: LIBS += -L/usr/local/lib/ -lswscale
unix:!macx: LIBS += -L/usr/local/lib/ -lavutil
unix:!macx: LIBS += -L/usr/local/lib/ -lavcodec

INCLUDEPATH +=/usr/local/include/Anjian
DEPENDPATH += /usr/local/include/Anjian

INCLUDEPATH += /usr/local/include/Anjian/third_party/
DEPENDPATH += /usr/local/include/Anjian/third_party/

INCLUDEPATH += /usr/local/include/Anjian/adapter/
DEPENDPATH += /usr/local/include/Anjian/adapter/

INCLUDEPATH += /usr/local/include/Anjian/third_party/libyolo/
DEPENDPATH += /usr/local/include/Anjian/third_party/libyolo/

INCLUDEPATH += /usr/local/include/Anjian/third_party/imagerunner/
DEPENDPATH += /usr/local/include/Anjian/third_party/imagerunner/


INCLUDEPATH += /usr/local/include/Anjian/third_party/utils/
DEPENDPATH += /usr/local/include/Anjian/third_party/utils/

unix:!macx: LIBS += -L/usr/local/lib/ -ladapter
unix:!macx: LIBS += -L/usr/local/lib/ -limgrunner
unix:!macx: LIBS += -L/usr/local/lib/ -lyolo
unix:!macx: LIBS += -L/usr/local/lib/ -lscheduler

unix:!macx: LIBS += -L/usr/local/lib/ -lajutils
