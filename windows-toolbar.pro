QT      += gui widgets multimedia winextras

TARGET   = $$qtLibraryTarget(windows-toolbar)
TEMPLATE = lib

MiamPlayerBuildDirectory = C:\dev\Madame-Miam-Miam-Music-Player\build\MiamPlayer

DEFINES += MIAM_PLUGIN

CONFIG  += dll c++11
CONFIG(debug, debug|release) {
    target.path = $$MiamPlayerBuildDirectory\debug\plugins
    LIBS += -Ldebug -lMiamCore
}

CONFIG(release, debug|release) {
    target.path = $$MiamPlayerBuildDirectory\release\plugins
    LIBS += -Lrelease -lMiamCore
}

INSTALLS += target

HEADERS += basicplugininterface.h \
    cover.h \
    filehelper.h \
    mediaplayer.h \
    mediaplayerplugininterface.h \
    miamcore_global.h \
    settings.h \
    windowstoolbar.h

SOURCES += windowstoolbar.cpp

FORMS += config.ui

RESOURCES += \
    resources.qrc
