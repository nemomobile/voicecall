include(../../plugin.pri)
TARGET = voicecall-ngf-plugin

PKGCONFIG += ngf-qt5

DEFINES += PLUGIN_NAME=\\\"ngf-plugin\\\"

#DEFINES += WANT_TRACE

HEADERS += \
    ngfringtoneplugin.h

SOURCES += \
    ngfringtoneplugin.cpp

