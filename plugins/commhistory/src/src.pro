include(../../plugin.pri)
QT += contacts dbus
TARGET = voicecall-commhistory-plugin

PKGCONFIG += qtcontacts-sqlite-qt5-extensions contactcache-qt5 commhistory-qt5 nemonotifications-qt5

DEFINES += PLUGIN_NAME=\\\"commhistory-plugin\\\"

#DEFINES += WANT_TRACE

HEADERS += \
    commhistoryplugin.h \
    calleventhandler.h

SOURCES += \
    commhistoryplugin.cpp \
    calleventhandler.cpp
