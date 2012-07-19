TEMPLATE = lib
TARGET = voicecall

QT = core dbus
CONFIG += qdbus

HEADERS += \
    common.h \
    voicecallmanagerinterface.h \
    abstractnotificationprovider.h \
    abstractvoicecallhandler.h \
    abstractvoicecallprovider.h \
    abstractvoicecallmanagerplugin.h \
    dbus/voicecallmanagerdbusproxy.h \
    dbus/voicecallmanagerdbusadapter.h \
    dbus/voicecallhandlerdbusproxy.h \
    dbus/voicecallhandlerdbusadapter.h \
    voicecallprovidermodel.h \
    voicecallmodel.h

SOURCES += \
    dbus/voicecallmanagerdbusproxy.cpp \
    dbus/voicecallmanagerdbusadapter.cpp \
    dbus/voicecallhandlerdbusproxy.cpp \
    dbus/voicecallhandlerdbusadapter.cpp \
    voicecallprovidermodel.cpp \
    voicecallmodel.cpp

target.path = /usr/lib

INSTALLS += target
