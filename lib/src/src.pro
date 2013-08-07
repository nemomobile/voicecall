TEMPLATE = lib
TARGET = voicecall

QT = core dbus

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
    dbus/voicecallhandlerdbusadapter.cpp \
    abstractvoicecallhandler.cpp

target.path = /usr/lib

INSTALLS += target

OTHER_FILES +=
