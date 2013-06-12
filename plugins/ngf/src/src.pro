include(../../plugin.pri)
TARGET = voicecall-ngf-plugin

PKGCONFIG += ngf-qt5

DEFINES += PLUGIN_NAME=\\\"ngf-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

#DEFINES += WANT_TRACE

HEADERS += \
    ngfringtoneplugin.h

SOURCES += \
    ngfringtoneplugin.cpp

