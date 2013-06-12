TARGET = voicecall-manager
TEMPLATE = app
QT = core dbus
CONFIG += link_pkgconfig

#DEFINES += WANT_TRACE

INCLUDEPATH += ../lib/src

PKGCONFIG += libresourceqt5
QT += multimedia # for basic ringtone plugin.
LIBS += -L../lib/src -lvoicecall

HEADERS += \
    dbus/voicecallmanagerdbusservice.h \
    basicvoicecallconfigurator.h \
    audiocallpolicyproxy.h \
    voicecallmanager.h \
    basicringtonenotificationprovider.h

SOURCES += \
    dbus/voicecallmanagerdbusservice.cpp \
    basicvoicecallconfigurator.cpp \
    audiocallpolicyproxy.cpp \
    voicecallmanager.cpp \
    main.cpp \
    basicringtonenotificationprovider.cpp

target.path = /usr/bin

INSTALLS += target

OTHER_FILES += voicecall-manager.desktop voicecall-manager.service

autostart_entry.files = voicecall-manager.desktop
autostart_entry.path = /etc/xdg/autostart

systemd_service_entry.files = voicecall-manager.service
systemd_service_entry.path = /usr/lib/systemd/user

INSTALLS += autostart_entry systemd_service_entry

