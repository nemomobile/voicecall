include(../qtsingleapplication/src/qtsinglecoreapplication.pri)

TARGET = voicecall-manager
TEMPLATE = app
QT = core dbus
CONFIG += mobility link_pkgconfig

DEFINES += WANT_TRACE

INCLUDEPATH += ../lib/src
LIBS += -L../lib/src -lvoicecall

CONFIG += qdbus # for dbus plugin.
MOBILITY = multimedia # for basic ringtone plugin.

HEADERS += \
    common.h \
    dbus/voicecallmanagerdbusservice.h \
    basicvoicecallconfigurator.h \
    basicringtonenotificationprovider.h \
    voicecallmanager.h

SOURCES += \
    dbus/voicecallmanagerdbusservice.cpp \
    basicringtonenotificationprovider.cpp \
    basicvoicecallconfigurator.cpp \
    voicecallmanager.cpp \
    main.cpp

target.path = /usr/bin

INSTALLS += target
