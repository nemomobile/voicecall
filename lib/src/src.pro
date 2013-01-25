TEMPLATE = lib
TARGET = voicecall

QT = core dbus
CONFIG += qdbus

#DEFINES += WANT_TRACE

HEADERS += \
    common.h \
    voicecallmanagerinterface.h \
    abstractnotificationprovider.h \
    abstractvoicecallhandler.h \
    abstractvoicecallprovider.h \
    abstractvoicecallmanagerplugin.h \
    dbus/voicecallmanagerdbusadapter.h \
    dbus/voicecallhandlerdbusadapter.h

SOURCES += \
    dbus/voicecallmanagerdbusadapter.cpp \
    dbus/voicecallhandlerdbusadapter.cpp

target.path = /usr/lib

INSTALLS += target

OTHER_FILES +=
