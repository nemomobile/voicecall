include(../../plugin.pri)
TARGET = voicecall-telepathy-plugin

PKGCONFIG += TelepathyQt5 TelepathyQt5Farstream

#DEFINES += WANT_TRACE

HEADERS += \
    telepathyproviderplugin.h \
    telepathyprovider.h \
    telepathyhandler.h \
    farstreamchannel.h

SOURCES += \
    telepathyproviderplugin.cpp \
    telepathyprovider.cpp \
    telepathyhandler.cpp \
    farstreamchannel.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-telepathy-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.1.0\\\"
