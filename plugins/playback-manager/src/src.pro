TEMPLATE = lib
TARGET = voicecall-playback-manager-plugin

QT = core dbus
CONFIG += plugin

DEFINES += PLUGIN_NAME=\\\"voicecall-playback-manager-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

DEFINES += WANT_TRACE

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

HEADERS += \
    common.h \
    playbackmanagerplugin.h

SOURCES += \
    playbackmanagerplugin.cpp

target.path = /usr/lib/voicecall/plugins

INSTALLS += target
