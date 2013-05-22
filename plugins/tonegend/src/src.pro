TEMPLATE = lib
TARGET = voicecall-tonegend-plugin

QT = core dbus
CONFIG += plugin link_pkgconfig

#DEFINES += WANT_TRACE

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

HEADERS += \
    tonegeneratorplugin.h

SOURCES += \
    tonegeneratorplugin.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-tonegend-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

target.path = /usr/lib/voicecall/plugins

INSTALLS += target
