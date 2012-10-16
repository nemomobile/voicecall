TEMPLATE = subdirs
CONFIG = ordered
SUBDIRS = lib plugins src ui

OTHER_FILES = LICENSE *.desktop packaging/voicecall.spec.in makedist voicecall-manager.service

autostart_entry.files = voicecall-manager.desktop
autostart_entry.path = /etc/xdg/autostart

systemd_service_entry.files = voicecall-manager.service
systemd_service_entry.path = /lib/systemd/user

INSTALLS += autostart_entry systemd_service_entry
