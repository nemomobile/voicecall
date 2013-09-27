#include "voicecallplugin.h"

#include "voicecallhandler.h"
#include "voicecallmanager.h"

#include "voicecallmodel.h"
#include "voicecallprovidermodel.h"

#include <QtQml>

void VoiceCallPlugin::registerTypes(const char *uri)
{
    qmlRegisterUncreatableType<VoiceCallHandler>(uri, 1, 0, "VoiceCall", "uncreatable type");
    qmlRegisterUncreatableType<VoiceCallModel>(uri, 1, 0, "VoiceCallModel", "uncreatable type");
    qmlRegisterUncreatableType<VoiceCallProviderModel>(uri, 1, 0, "VoiceCallProviderModel", "uncreatable type");

    qmlRegisterType<VoiceCallManager>(uri, 1, 0, "VoiceCallManager");
}

