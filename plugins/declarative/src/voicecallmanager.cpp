#include "common.h"
#include "voicecallmanager.h"

#include <NgfClient>

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
          voicecalls(NULL),
          providers(NULL),
          activeVoiceCall(NULL),
          ngf(0),
          eventId(0),
          connected(false)
    { /*...*/ }

    VoiceCallManager *q_ptr;

    QDBusInterface *interface;

    VoiceCallModel *voicecalls;
    VoiceCallProviderModel *providers;

    VoiceCallHandler* activeVoiceCall;

    Ngf::Client *ngf;
    quint32 eventId;

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

    d->ngf = new Ngf::Client(this);
    d->ngf->connect();

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
    if(d->providers->count() == 0) {
        qWarning() << Q_FUNC_INFO << "No provider added";
        return QString();
    }

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
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingSilenceFinished(QDBusPendingCallWatcher*)));
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

void VoiceCallManager::setAudioModeAsync(const QString &mode)
{
    TRACE
    Q_D(const VoiceCallManager);
    d->interface->asyncCall("setAudioMode", mode);
}

void VoiceCallManager::setAudioRoutedAsync(bool on)
{
    TRACE
    Q_D(const VoiceCallManager);
    d->interface->asyncCall("setAudioRouted", on);
}

void VoiceCallManager::setMuteMicrophoneAsync(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->interface->asyncCall("setMuteMicrophone", on);
}

void VoiceCallManager::setMuteSpeakerAsync(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->interface->asyncCall("setMuteSpeaker", on);
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

    QMap<QString, QVariant> properties;
    properties.insert("tonegen.value", toneId);
    if (d->eventId > 0)
    {
        d->ngf->stop(d->eventId);
    }
    d->eventId = d->ngf->play("dtmf", properties);

    return true;
}

bool VoiceCallManager::stopDtmfTone()
{
    TRACE
    Q_D(VoiceCallManager);

    if (d->eventId > 0)
    {
        d->ngf->stop(d->eventId);
        d->eventId = 0;
    }

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

    if (reply.isError()) {
        emit this->error(reply.error().message());
    } else {
        DEBUG_T(QString("Received successful reply for member: ") + reply.reply().member());
    }

    watcher->deleteLater();
}

void VoiceCallManager::onPendingSilenceFinished(QDBusPendingCallWatcher *watcher)
{
    TRACE
    QDBusPendingReply<> reply = *watcher;

    if (reply.isError()) {
        emit this->error(reply.error().message());
    } else {
        DEBUG_T(QString("Received successful reply for member: ") + reply.reply().member());
    }

    watcher->deleteLater();
}
