#-------------------------------------------------
#
# Project created by QtCreator 2013-11-13T20:19:11
#
#-------------------------------------------------

QT      += gui widgets uitools

TARGET   = $$qtLibraryTarget(windows-toolbar)
TEMPLATE = lib
CONFIG  += plugin

LIBS += libcomctl32 libole32

SOURCES += windowstoolbar.cpp

HEADERS += windowstoolbar.h \
    basicplugininterface.h \
    mediaplayerplugininterface.h \
    win7_include.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    config.ui

RESOURCES += \
    ressources.qrc
