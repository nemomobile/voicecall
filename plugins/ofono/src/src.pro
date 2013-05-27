TEMPLATE = lib
TARGET = voicecall-ofono-plugin

QT = core dbus
CONFIG += plugin link_pkgconfig

#DEFINES += WANT_TRACE

INCLUDEPATH += ../../../lib/src
LIBS += -L../../../lib/src -lvoicecall

PKGCONFIG += qofono

HEADERS += \
    common.h \
    ofonovoicecallhandler.h  \
    ofonovoicecallprovider.h \
    ofonovoicecallproviderfactory.h

SOURCES += \
    ofonovoicecallhandler.cpp \
    ofonovoicecallprovider.cpp \
    ofonovoicecallproviderfactory.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-ofono-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

target.path = /usr/lib/voicecall/plugins

INSTALLS += target

