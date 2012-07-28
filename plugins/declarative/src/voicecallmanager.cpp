#include "common.h"
#include "voicecallmanager.h"

#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingReply>

class VoiceCallManagerPrivate
{
public:
    VoiceCallManagerPrivate()
        : interface(NULL),
          voicecalls(NULL),
          providers(NULL),
          activeVoiceCall(NULL),
          connected(false)
    { /*...*/ }

    QDBusInterface *interface;

    VoiceCallModel *voicecalls;
    VoiceCallProviderModel *providers;

    VoiceCallHandler* activeVoiceCall;

    bool connected;
};

VoiceCallManager::VoiceCallManager(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), d(new VoiceCallManagerPrivate)
{
    TRACE
    d->interface = new QDBusInterface("stage.rubyx.voicecall",
                                      "/",
                                      "stage.rubyx.voicecall.VoiceCallManager",
                                      QDBusConnection::sessionBus(),
                                      this);

    d->voicecalls = new VoiceCallModel(this);
    d->providers = new VoiceCallProviderModel(this);

    this->initialize();
}

VoiceCallManager::~VoiceCallManager()
{
    TRACE
    delete this->d;
}

void VoiceCallManager::initialize(bool notifyError)
{
    TRACE
    bool success = false;

    if(d->interface->isValid())
    {
        success = true;
        success &= QObject::connect(d->interface, SIGNAL(error(QString)), SIGNAL(error(QString)));
        success &= QObject::connect(d->interface, SIGNAL(voiceCallsChanged()), SLOT(onVoiceCallsChanged()));
        success &= QObject::connect(d->interface, SIGNAL(providersChanged()), SLOT(onProvidersChanged()));
        success &= QObject::connect(d->interface, SIGNAL(activeVoiceCallChanged()), SLOT(onActiveVoiceCallChanged()));

        this->onActiveVoiceCallChanged();
    }

    if(!(d->connected = success))
    {
        QTimer::singleShot(2000, this, SLOT(initialize()));
        if(notifyError) emit this->error("Failed to connect to VCM D-Bus service.");
    }
}

QDBusInterface* VoiceCallManager::interface() const
{
    TRACE
    return d->interface;
}

VoiceCallModel* VoiceCallManager::voiceCalls() const
{
    TRACE
    return d->voicecalls;
}

VoiceCallProviderModel* VoiceCallManager::providers() const
{
    TRACE
    return d->providers;
}

VoiceCallHandler* VoiceCallManager::activeVoiceCall() const
{
    TRACE
    return d->activeVoiceCall;
}

void VoiceCallManager::dial(const QString &provider, const QString &msisdn)
{
    TRACE
    QDBusPendingCall call = d->interface->asyncCall("dial", provider, msisdn);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

bool VoiceCallManager::setMuteRingtone(bool on)
{
    TRACE
    QDBusPendingReply<bool> reply = d->interface->call("setMuteRingtone", on);
    return reply.isError() ? false : reply.value();
}

bool VoiceCallManager::startDtmfTone(const QString &tone)
{
    TRACE
    QDBusPendingReply<bool> reply = d->interface->call("startDtmfTone", tone);
    return reply.isError() ? false : reply.value();
}

bool VoiceCallManager::stopDtmfTone()
{
    TRACE
    QDBusPendingReply<bool> reply = d->interface->call("stopDtmfTone");
    return reply.isError() ? false : reply.value();
}

void VoiceCallManager::onVoiceCallsChanged()
{
    TRACE
    emit this->voiceCallsChanged();
}

void VoiceCallManager::onProvidersChanged()
{
    TRACE
    emit this->providersChanged();
}

void VoiceCallManager::onActiveVoiceCallChanged()
{
    TRACE
    QString voiceCallId = d->interface->property("activeVoiceCall").toString();

    if(voiceCallId.isNull() || voiceCallId.isEmpty())
    {
        d->activeVoiceCall = NULL;
    }
    else
    {
        d->activeVoiceCall = d->voicecalls->instance(voiceCallId);
    }

    emit this->activeVoiceCallChanged();
}

void VoiceCallManager::onPendingCallFinished(QDBusPendingCallWatcher *watcher)
{
    TRACE
    QDBusPendingReply<bool> reply = *watcher;

    if(reply.isError())
    {
        emit this->error(reply.error().message());
        watcher->deleteLater();
        return;
    }

    DEBUG_T(QString("Received successful reply for member: ") + reply.reply().member());
}
