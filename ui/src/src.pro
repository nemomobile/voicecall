include(../../qtsingleapplication/src/qtsingleapplication.pri)

TARGET = voicecall-ui
TEMPLATE = app
QT = core network gui declarative

INCLUDEPATH += ../../lib/src
LIBS += -L../../lib/src -lvoicecall

DEFINES += WANT_TRACE

HEADERS += \
    common.h \
    declarativeview.h

SOURCES += \
    main.cpp \
    declarativeview.cpp

target.path = /usr/bin

INSTALLS += target
