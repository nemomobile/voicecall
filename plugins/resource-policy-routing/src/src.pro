include(../../plugin.pri)
TARGET = voicecall-resource-policy-routing-plugin
QT += dbus

DEFINES += PLUGIN_NAME=\\\"resource-policy-routing-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

#DEFINES += WANT_TRACE

HEADERS += \
    resourcepolicyroutingplugin.h

SOURCES += \
    resourcepolicyroutingplugin.cpp

