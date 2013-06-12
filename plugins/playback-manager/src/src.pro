include(../../plugin.pri)
TARGET = voicecall-playback-manager-plugin
QT += dbus

DEFINES += PLUGIN_NAME=\\\"voicecall-playback-manager-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

#DEFINES += WANT_TRACE

HEADERS += \
    playbackmanagerplugin.h

SOURCES += \
    playbackmanagerplugin.cpp
