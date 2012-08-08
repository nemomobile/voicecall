TEMPLATE = subdirs
CONFIG = ordered
SUBDIRS = lib plugins src ui

OTHER_FILES = LICENSE *.desktop packaging/voicecall.spec.in makedist

autostart_entry.files = voicecall-manager.desktop
autostart_entry.path = /etc/xdg/autostart

INSTALLS += autostart_entry
