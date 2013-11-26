QT      += widgets multimedia winextras

QMAKE_CXXFLAGS += -std=c++11

TARGET   = $$qtLibraryTarget(windows-toolbar)
TEMPLATE = lib
CONFIG  += plugin
CONFIG(debug, debug|release) {
    LIBS += -debug -lMiamCore
}

CONFIG(release, debug|release) {
    LIBS += -Lrelease -lMiamCore
}

HEADERS += windowstoolbar.h \
    basicplugininterface.h \
    mediaplayerplugininterface.h \
    settings.h \
    filehelper.h \
    miamcore_global.h

SOURCES += windowstoolbar.cpp


FORMS += config.ui

RESOURCES += resources.qrc
