#ifndef VOICECALLPLUGIN_H
#define VOICECALLPLUGIN_H

#include <QtGlobal>

#include <QtQml/QQmlExtensionPlugin>

class VoiceCallPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall")

public:
    void registerTypes(const char *uri);
};

#endif // VOICECALLPLUGIN_H
