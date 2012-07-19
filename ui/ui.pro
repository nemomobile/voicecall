TEMPLATE = subdirs
CONFIG = ordered
SUBDIRS = src

OTHER_FILES += qml/*.qml
OTHER_FILES += LICENSE voicecall-ui.yaml *.desktop

autostart_entry.files = voicecall-ui-prestart.desktop
autostart_entry.path = /etc/xdg/autostart

desktopfile_entry.files = voicecall-ui.desktop
desktopfile_entry.path = /usr/share/applications

qml.files = qml
qml.path = /usr/share/voicecall-ui

INSTALLS += qml autostart_entry desktopfile_entry
