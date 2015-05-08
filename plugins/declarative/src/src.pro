CONFIG += no_libvoicecall no_plugininstall
include(../../plugin.pri)
QT = core dbus qml

TARGET = voicecall
uri = org.nemomobile.voicecall

#DEFINES += WANT_TRACE

PKGCONFIG += ngf-qt5

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
installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
qmldir.path = $$installPath
target.path = $$installPath
INSTALLS += target qmldir
