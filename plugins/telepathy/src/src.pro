TEMPLATE = lib
TARGET = voicecall-telepathy-plugin

QT = core
CONFIG += plugin link_pkgconfig

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

PKGCONFIG += TelepathyQt4 TelepathyQt4Farstream

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

target.path = /usr/lib/voicecall/plugins

INSTALLS += target
