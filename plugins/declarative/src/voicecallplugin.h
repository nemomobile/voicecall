#ifndef VOICECALLPLUGIN_H
#define VOICECALLPLUGIN_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
 #include <QtQml/QQmlExtensionPlugin>
 #define QDeclarativeExtensionPlugin QQmlExtensionPlugin
#else
# include <QtDeclarative/QDeclarativeExtensionPlugin>
#endif

class VoiceCallPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall")
#endif

public:
    void registerTypes(const char *uri);
};

#endif // VOICECALLPLUGIN_H
