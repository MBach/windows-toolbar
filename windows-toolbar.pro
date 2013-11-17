QT      += widgets uitools multimedia winextras

QMAKE_CXXFLAGS += -std=c++11

TARGET   = $$qtLibraryTarget(windows-toolbar)
TEMPLATE = lib
CONFIG  += plugin

HEADERS += windowstoolbar.h \
    basicplugininterface.h \
    mediaplayerplugininterface.h

SOURCES += windowstoolbar.cpp

FORMS += config.ui

RESOURCES += resources.qrc
