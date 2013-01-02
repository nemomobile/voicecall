TEMPLATE = lib
TARGET = voicecall-ngf-plugin

QT = core
CONFIG += plugin link_pkgconfig

PKGCONFIG += ngf-qt

DEFINES += PLUGIN_NAME=\\\"ngf-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

DEFINES += WANT_TRACE

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

HEADERS += \
    common.h \
    ngfringtoneplugin.h

SOURCES += \
    ngfringtoneplugin.cpp

target.path = /usr/lib/voicecall/plugins

INSTALLS += target
