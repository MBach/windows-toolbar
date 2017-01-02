QT      += gui widgets multimedia sql winextras

TARGET   = $$qtLibraryTarget(windows-toolbar)
TEMPLATE = lib

DEFINES += MIAM_PLUGIN

CONFIG  += dll c++11
CONFIG(debug, debug|release) {
    LIBS += -L$$OUT_PWD/../../core/debug -lmiam-core
}

CONFIG(release, debug|release) {
    LIBS += -L$$OUT_PWD/../../core/release -lmiam-core
}

DESTDIR += $$OUT_PWD/../../player/release/plugins

HEADERS += interfaces/basicplugin.h \
    interfaces/mediaplayerplugin.h \
    model/genericdao.h \
    model/playlistdao.h \
    model/sqldatabase.h \
    model/trackdao.h \
    abstractmediaplayercontrol.h \
    cover.h \
    filehelper.h \
    mediaplayer.h \
    mediaplaylist.h \
    miamcore_global.h \
    musicsearchengine.h \
    settings.h \
    windowstoolbar.h

SOURCES += windowstoolbar.cpp

FORMS += config.ui

RESOURCES += resources.qrc

TRANSLATIONS += translations/WindowsToolBar_ar.ts \
    translations/WindowsToolBar_cs.ts \
    translations/WindowsToolBar_de.ts \
    translations/WindowsToolBar_en.ts \
    translations/WindowsToolBar_es.ts \
    translations/WindowsToolBar_fr.ts \
    translations/WindowsToolBar_it.ts \
    translations/WindowsToolBar_ja.ts \
    translations/WindowsToolBar_kr.ts \
    translations/WindowsToolBar_pt.ts \
    translations/WindowsToolBar_ru.ts \
    translations/WindowsToolBar_th.ts \
    translations/WindowsToolBar_vn.ts \
    translations/WindowsToolBar_zh.ts
