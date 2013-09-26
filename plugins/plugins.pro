TEMPLATE = subdirs

# Qt 4 doesn't have the voicecall daemon, so the plugins for it aren't required
# either. We do build the declarative voicecall plugin so that a Qt 4 UI can
# interact with the daemon though.
#
# Qt 5's voiceall needs all plugins.
equals(QT_MAJOR_VERSION, 4): SUBDIRS = declarative
equals(QT_MAJOR_VERSION, 5): SUBDIRS = declarative commhistory providers resource-policy-routing ngf playback-manager
