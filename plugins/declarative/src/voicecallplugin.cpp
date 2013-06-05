#include "voicecallplugin.h"

#include "voicecallhandler.h"
#include "voicecallmanager.h"

#include "voicecallmodel.h"
#include "voicecallprovidermodel.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QtQml>
#else
# include <QtDeclarative>
#endif

void VoiceCallPlugin::registerTypes(const char *uri)
{
    // @uri stage.rubyx.voicecall
    qmlRegisterUncreatableType<VoiceCallHandler>(uri, 1, 0, "VoiceCall", "uncreatable type");
    qmlRegisterUncreatableType<VoiceCallModel>(uri, 1, 0, "VoiceCallModel", "uncreatable type");
    qmlRegisterUncreatableType<VoiceCallProviderModel>(uri, 1, 0, "VoiceCallProviderModel", "uncreatable type");

    qmlRegisterType<VoiceCallManager>(uri, 1, 0, "VoiceCallManager");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(voicecall, VoiceCallPlugin)
#endif
