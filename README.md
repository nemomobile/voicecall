# voicecall

voicecall is a daemon (and QML plugin) for implementing dialer UIs.

## voicecall-manager

voicecall-manager is a system daemon responsible for collating and managing 
the telephony subsystem for the dialer UI to interact with.

the dialer instructs voicecall-manager to make calls, which then subsequently
uses plugins (such as an ofono backend, for instance) to make the actual call.

likewise, the daemon listens to various backends in order to communicate
incoming calls to the user interface.

## dbus

communication between the user interface and the manager occurs over dbus, on
the org.nemomobile.voicecall session bus interface.
