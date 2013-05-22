TARGET = voicecall-ui
TEMPLATE = app
QT = core network gui declarative dbus

CONFIG += link_pkgconfig

packagesExist(qdeclarative-boostable) {
    message("Building with qdeclarative-boostable support")
    DEFINES += HAS_BOOSTER
    PKGCONFIG += qdeclarative-boostable
} else {
    warning("qdeclarative-boostable not available; startup times will be slower")
}

#DEFINES += WANT_TRACE

DEFINES += APPLICATION_NAME=\\\"voicecall-ui\\\"
DEFINES += APPLICATION_VERSION=\\\"0.1.0\\\"
DEFINES += APPLICATION_ORGANISATION=\\\"stage.rubyx.co.uk\\\"
DEFINES += APPLICATION_DOMAIN=\\\"stage.rubyx.co.uk\\\"

HEADERS += \
    common.h \
    declarativeview.h \
    dbusadaptor.h

SOURCES += \
    main.cpp \
    declarativeview.cpp \
    dbusadaptor.cpp

target.path = /usr/bin

INSTALLS += target
