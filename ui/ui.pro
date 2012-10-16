TEMPLATE = subdirs
CONFIG = ordered
SUBDIRS = src

OTHER_FILES += qml/*.qml
OTHER_FILES += LICENSE voicecall-ui.yaml *.desktop *.service

autostart_entry.files = voicecall-ui-prestart.desktop
autostart_entry.path = /etc/xdg/autostart

desktopfile_entry.files = voicecall-ui.desktop
desktopfile_entry.path = /usr/share/applications

systemd_service_entry.files = voicecall-ui-prestart.service
systemd_service_entry.path = /usr/lib/systemd/user

qml.files = qml
qml.path = /usr/share/voicecall-ui

INSTALLS += qml desktopfile_entry autostart_entry systemd_service_entry
