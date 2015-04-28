include(../../plugin.pri)
TARGET = voicecall-commhistory-plugin

PKGCONFIG += commhistory-qt5

DEFINES += PLUGIN_NAME=\\\"commhistory-plugin\\\"

#DEFINES += WANT_TRACE

HEADERS += \
    commhistoryplugin.h \
    calleventhandler.h

SOURCES += \
    commhistoryplugin.cpp \
    calleventhandler.cpp
