include(../../plugin.pri)
TARGET = voicecall-tonegend-plugin
QT += dbus

#DEFINES += WANT_TRACE

HEADERS += \
    tonegeneratorplugin.h

SOURCES += \
    tonegeneratorplugin.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-tonegend-plugin\\\"
