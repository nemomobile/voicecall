#ifndef VOICECALLPLUGIN_H
#define VOICECALLPLUGIN_H

#include <QtDeclarative/QDeclarativeExtensionPlugin>

class VoiceCallPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
    
public:
    void registerTypes(const char *uri);
};

#endif // VOICECALLPLUGIN_H
