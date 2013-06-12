CONFIG += no_libvoicecall no_plugininstall
include(../../plugin.pri)
QT = core dbus
equals(QT_MAJOR_VERSION, 4): QT += declarative
equals(QT_MAJOR_VERSION, 5): QT += qml

TARGET = voicecall
equals(QT_MAJOR_VERSION, 4): uri = stage.rubyx.voicecall
equals(QT_MAJOR_VERSION, 5): uri = org.nemomobile.voicecall

#DEFINES += WANT_TRACE

HEADERS += \
    voicecallhandler.h \
    voicecallmanager.h \
    voicecallmodel.h \
    voicecallprovidermodel.h \
    voicecallplugin.h

SOURCES += \
    voicecallhandler.cpp \
    voicecallmanager.cpp \
    voicecallmodel.cpp \
    voicecallprovidermodel.cpp \
    voicecallplugin.cpp

OTHER_FILES += qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
equals(QT_MAJOR_VERSION, 4): installPath = $$[QT_INSTALL_IMPORTS]/$$replace(uri, \\., /)
equals(QT_MAJOR_VERSION, 5): installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
qmldir.path = $$installPath
target.path = $$installPath
INSTALLS += target qmldir
