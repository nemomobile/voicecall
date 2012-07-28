include(../../qtsingleapplication/src/qtsingleapplication.pri)

TARGET = voicecall-ui
TEMPLATE = app
QT = core network gui declarative

DEFINES += WANT_TRACE

HEADERS += \
    common.h \
    declarativeview.h

SOURCES += \
    main.cpp \
    declarativeview.cpp

target.path = /usr/bin

INSTALLS += target
