#include "common.h"
#include "voicecallmanager.h"

#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingReply>

class VoiceCallManagerPrivate
{
    Q_DECLARE_PUBLIC(VoiceCallManager)

public:
    VoiceCallManagerPrivate(VoiceCallManager *q)
        : q_ptr(q),
          interface(NULL),
          tonegend(NULL),
          voicecalls(NULL),
          providers(NULL),
          activeVoiceCall(NULL),
          connected(false)
    { /*...*/ }

    VoiceCallManager *q_ptr;

    QDBusInterface *interface;
    QDBusInterface *tonegend;

    VoiceCallModel *voicecalls;
    VoiceCallProviderModel *providers;

    VoiceCallHandler* activeVoiceCall;

    bool connected;
};

VoiceCallManager::VoiceCallManager(QObject *parent)
    : QObject(parent), d_ptr(new VoiceCallManagerPrivate(this))
{
    TRACE
    Q_D(VoiceCallManager);
    d->interface = new QDBusInterface("org.nemomobile.voicecall",
                                      "/",
                                      "org.nemomobile.voicecall.VoiceCallManager",
                                      QDBusConnection::sessionBus(),
                                      this);
    d->tonegend = new QDBusInterface("com.Nokia.Telephony.Tones",
                                     "/com/Nokia/Telephony/Tones",
                                     "com.Nokia.Telephony.Tones",
                                     QDBusConnection::systemBus(),
                                     this);

    d->voicecalls = new VoiceCallModel(this);
    d->providers = new VoiceCallProviderModel(this);

    this->initialize();
}

VoiceCallManager::~VoiceCallManager()
{
    TRACE
    Q_D(VoiceCallManager);
    delete d;
}

void VoiceCallManager::initialize(bool notifyError)
{
    TRACE
    Q_D(VoiceCallManager);
    bool success = false;

    if(d->interface->isValid())
    {
        success = true;
        success &= (bool)QObject::connect(d->interface, SIGNAL(error(QString)), SIGNAL(error(QString)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(voiceCallsChanged()), SLOT(onVoiceCallsChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(providersChanged()), SLOT(onProvidersChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(activeVoiceCallChanged()), SLOT(onActiveVoiceCallChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(audioModeChanged()), SIGNAL(audioModeChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(audioRoutedChanged()), SIGNAL(audioRoutedChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(microphoneMutedChanged()), SIGNAL(microphoneMutedChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(speakerMutedChanged()), SIGNAL(speakerMutedChanged()));

        onActiveVoiceCallChanged();
        onVoiceCallsChanged();
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
    Q_D(const VoiceCallManager);
    return d->interface;
}

VoiceCallModel* VoiceCallManager::voiceCalls() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->voicecalls;
}

VoiceCallProviderModel* VoiceCallManager::providers() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->providers;
}

QString VoiceCallManager::defaultProviderId() const
{
    TRACE
    Q_D(const VoiceCallManager);
    if(d->providers->count() == 0) return QString::null;
    return d->providers->id(0); //TODO: Add support for select default voice call provider.
}

VoiceCallHandler* VoiceCallManager::activeVoiceCall() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->activeVoiceCall;
}

QString VoiceCallManager::audioMode() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->interface->property("audioMode").toString();
}

bool VoiceCallManager::isAudioRouted() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->interface->property("isAudioRouted").toBool();
}

bool VoiceCallManager::isMicrophoneMuted() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->interface->property("isMicrophoneMuted").toBool();
}

bool VoiceCallManager::isSpeakerMuted() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->interface->property("isSpeakerMuted").toBool();
}

void VoiceCallManager::dial(const QString &provider, const QString &msisdn)
{
    TRACE
    Q_D(VoiceCallManager);
    QDBusPendingCall call = d->interface->asyncCall("dial", provider, msisdn);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallManager::silenceRingtone()
{
    TRACE
    Q_D(const VoiceCallManager);
    QDBusPendingCall call = d->interface->asyncCall("silenceRingtone");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*
  - Use of method calls instead of property setters to allow status checking.
 */
bool VoiceCallManager::setAudioMode(const QString &mode)
{
    TRACE
    Q_D(const VoiceCallManager);
    QDBusPendingReply<bool> reply = d->interface->call("setAudioMode", mode);
    return reply.isError() ? false : reply.value();
}

bool VoiceCallManager::setAudioRouted(bool on)
{
    TRACE
    Q_D(const VoiceCallManager);
    QDBusPendingReply<bool> reply = d->interface->call("setAudioRouted", on);
    return reply.isError() ? false : reply.value();
}

bool VoiceCallManager::setMuteMicrophone(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    QDBusPendingReply<bool> reply = d->interface->call("setMuteMicrophone", on);
    return reply.isError() ? false : reply.value();
}

bool VoiceCallManager::setMuteSpeaker(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    QDBusPendingReply<bool> reply = d->interface->call("setMuteSpeaker", on);
    return reply.isError() ? false : reply.value();
}

bool VoiceCallManager::startDtmfTone(const QString &tone)
{
    TRACE
    Q_D(VoiceCallManager);

    bool ok = true;
    unsigned int toneId = tone.toInt(&ok);

    if(!ok)
    {
        if (tone == "*") toneId = 10;
        else if(tone == "#") toneId = 11;
        else if(tone == "A") toneId = 12;
        else if(tone == "B") toneId = 13;
        else if(tone == "C") toneId = 14;
        else if(tone == "D") toneId = 15;
        else return false;
    }

    if(d->activeVoiceCall)
    {
        d->activeVoiceCall->sendDtmf(tone);
    }

    d->tonegend->call("StartEventTone", toneId, 0, (unsigned int)0);
    return true;
}

bool VoiceCallManager::stopDtmfTone()
{
    TRACE
    Q_D(VoiceCallManager);
    d->tonegend->call("StopTone");
    return true;
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
    Q_D(VoiceCallManager);
    QString voiceCallId = d->interface->property("activeVoiceCall").toString();

    if(d->voicecalls->rowCount(QModelIndex()) == 0 || voiceCallId.isNull() || voiceCallId.isEmpty())
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
