TEMPLATE = subdirs
SUBDIRS += src lib plugins

plugins.depends = lib
src.depends = lib

OTHER_FILES = LICENSE makedist rpm/voicecall-qt5.spec
