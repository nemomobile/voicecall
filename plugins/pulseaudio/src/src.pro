TEMPLATE = lib
TARGET = voicecall-pulseaudio-plugin

QT = core
CONFIG += plugin link_pkgconfig

DEFINES += WANT_TRACE

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

PKGCONFIG += libpulse-mainloop-glib

HEADERS += \
    common.h \
    pacontrol.h \
    pulseaudioroutingplugin.h

SOURCES += \
    pacontrol.cpp \
    pulseaudioroutingplugin.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-pulseaudio-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

target.path = /usr/lib/voicecall/plugins

INSTALLS += target
