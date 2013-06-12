include(../../plugin.pri)
TARGET = voicecall-ofono-plugin
QT += dbus

#DEFINES += WANT_TRACE

PKGCONFIG += qofono-qt5

HEADERS += \
    ofonovoicecallhandler.h  \
    ofonovoicecallprovider.h \
    ofonovoicecallproviderfactory.h

SOURCES += \
    ofonovoicecallhandler.cpp \
    ofonovoicecallprovider.cpp \
    ofonovoicecallproviderfactory.cpp

DEFINES += PLUGIN_NAME=\\\"voicecall-ofono-plugin\\\"
DEFINES += PLUGIN_VERSION=\\\"0.0.0.1\\\"

