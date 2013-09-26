include(../../plugin.pri)
TARGET = voicecall-pulseaudio-plugin

DEFINES += PLUGIN_NAME=\\\"voicecall-pulseaudio-plugin\\\"

#DEFINES += WANT_TRACE

PKGCONFIG += libpulse-mainloop-glib

HEADERS += \
    pacontrol.h \
    pulseaudioroutingplugin.h

SOURCES += \
    pacontrol.cpp \
    pulseaudioroutingplugin.cpp
