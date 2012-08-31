TEMPLATE = lib
TARGET = voicecall-telepathy-plugin

QT = core
CONFIG += plugin link_pkgconfig

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

PKGCONFIG += TelepathyQt4

DEFINES += WANT_TRACE

HEADERS += \
    common.h \
    telepathyproviderplugin.h \
    telepathyprovider.h \
    telepathyhandler.h

SOURCES += \
    telepathyproviderplugin.cpp \
    telepathyprovider.cpp \
    telepathyhandler.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-telepathy-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

target.path = /usr/lib/voicecall/plugins

INSTALLS += target
