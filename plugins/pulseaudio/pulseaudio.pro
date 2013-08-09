TEMPLATE = subdirs
SUBDIRS = src

OTHER_FILES += LICENSE modes.ini

config.files = modes.ini
config.path = /etc/voicecall/

INSTALLS += config
