TEMPLATE = subdirs

# daemon is only built for Qt 5
equals(QT_MAJOR_VERSION, 5) {
    SUBDIRS += src lib

    # Qt 4 only builds the declarative plugin which does not use libvoicecall.
    plugins.depends = lib
}

SUBDIRS += plugins

src.depends = lib

OTHER_FILES = LICENSE makedist
