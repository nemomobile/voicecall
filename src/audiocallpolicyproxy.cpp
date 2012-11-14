#include "common.h"
#include "audiocallpolicyproxy.h"

#include <QTimer>

class AudioCallPolicyProxyPrivate
{
    Q_DECLARE_PUBLIC(AudioCallPolicyProxy)

public:
    AudioCallPolicyProxyPrivate(AudioCallPolicyProxy *q, AbstractVoiceCallHandler *s)
        : q_ptr(q), subject(s),
          resourceSet(NULL), onAcquireReceiver(NULL), onAcquireMethod(QString::null)
    { /* ... */ }

    AudioCallPolicyProxy        *q_ptr;
    AbstractVoiceCallHandler    *subject;

    ResourcePolicy::ResourceSet *resourceSet;

    QObject *onAcquireReceiver;
    QString  onAcquireMethod;
};

AudioCallPolicyProxy::AudioCallPolicyProxy(AbstractVoiceCallHandler *subject, QObject *parent)
    : AbstractVoiceCallHandler(parent), d_ptr(new AudioCallPolicyProxyPrivate(this, subject))
{
    TRACE
    QObject::connect(subject, SIGNAL(statusChanged()), SIGNAL(statusChanged()));
    QObject::connect(subject, SIGNAL(lineIdChanged()), SIGNAL(lineIdChanged()));
    QObject::connect(subject, SIGNAL(durationChanged()), SIGNAL(durationChanged()));
    QObject::connect(subject, SIGNAL(emergencyChanged()), SIGNAL(emergencyChanged()));
    QObject::connect(subject, SIGNAL(multipartyChanged()), SIGNAL(multipartyChanged()));
}

AudioCallPolicyProxy::~AudioCallPolicyProxy()
{
    TRACE
    this->deleteResourceSet();

    delete d_ptr;
}

AbstractVoiceCallProvider* AudioCallPolicyProxy::provider() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->provider();
}

QString AudioCallPolicyProxy::handlerId() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->handlerId();
}

QString AudioCallPolicyProxy::lineId() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->lineId();
}

QDateTime AudioCallPolicyProxy::startedAt() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->startedAt();
}

int AudioCallPolicyProxy::duration() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->duration();
}

bool AudioCallPolicyProxy::isMultiparty() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->isMultiparty();
}

bool AudioCallPolicyProxy::isEmergency() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->isEmergency();
}

AbstractVoiceCallHandler::VoiceCallStatus AudioCallPolicyProxy::status() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->status();
}

QString AudioCallPolicyProxy::statusText() const
{
    TRACE
    Q_D(const AudioCallPolicyProxy);
    return d->subject->statusText();
}

void AudioCallPolicyProxy::answer()
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    this->invokeWithResources(d->subject, SLOT(answer()));
}

void AudioCallPolicyProxy::hangup()
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    d->subject->hangup();
    this->deleteResourceSet();
}

void AudioCallPolicyProxy::deflect(const QString &target)
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    d->subject->deflect(target);
}

void AudioCallPolicyProxy::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    d->subject->sendDtmf(tones);
}

void AudioCallPolicyProxy::invokeWithResources(QObject *receiver, const QString &method)
{
    TRACE
    Q_D(AudioCallPolicyProxy);

    d->onAcquireReceiver = receiver;
    d->onAcquireMethod = method;

    this->createResourceSet();
    d->resourceSet->acquire();
}

void AudioCallPolicyProxy::createResourceSet()
{
    TRACE
    Q_D(AudioCallPolicyProxy);

    if(d->resourceSet)
    {
        DEBUG_T("Resource set already configured");
        return;
    }

    d->resourceSet = new ResourcePolicy::ResourceSet("call", this, true, true);

    ResourcePolicy::AudioResource *audioResource = new ResourcePolicy::AudioResource("call");
    audioResource->setProcessID(QCoreApplication::applicationPid());
    audioResource->setStreamTag("media.name", "*");
    audioResource->setOptional(false);

    d->resourceSet->addResourceObject(audioResource);

    d->resourceSet->addResource(ResourcePolicy::AudioPlaybackType);
    d->resourceSet->addResource(ResourcePolicy::AudioRecorderType);

    this->connectResourceSet();

    d->resourceSet->initAndConnect();
}

void AudioCallPolicyProxy::deleteResourceSet()
{
    TRACE
    Q_D(AudioCallPolicyProxy);

    if(!d->resourceSet)
    {
        DEBUG_T("Resource set not configured");
        return;
    }

    d->resourceSet->release();
    this->disconnectResourceSet();
    delete d->resourceSet;
    d->resourceSet = NULL;
}

void AudioCallPolicyProxy::connectResourceSet()
{
    TRACE
    Q_D(AudioCallPolicyProxy);

    if(!d->resourceSet)
    {
        DEBUG_T("Resource set not configured.");
        return;
    }

    DEBUG_T("Connecting resource set signals to slots.");
    QObject::connect(d->resourceSet,
                     SIGNAL(resourcesGranted(QList<ResourcePolicy::ResourceType>)),
                     SLOT(onResourceSetGranted()));
    QObject::connect(d->resourceSet,
                     SIGNAL(resourcesDenied()),
                     SLOT(onResourceSetDenied()));
    QObject::connect(d->resourceSet,
                     SIGNAL(lostResources()),
                     SLOT(onResourceSetLost()));
    QObject::connect(d->resourceSet,
                     SIGNAL(errorCallback(quint32, const char*)),
                     SLOT(onResourceSetError(quint32, const char*)));
    QObject::connect(d->resourceSet,
                     SIGNAL(resourcesBecameAvailable(QList<ResourcePolicy::ResourceType>)),
                     SLOT(onResourceSetBecameAvailable(QList<ResourcePolicy::ResourceType>)));
    QObject::connect(d->resourceSet,
                     SIGNAL(resourcesReleased()),
                     SLOT(onResourceSetReleased()));
    QObject::connect(d->resourceSet,
                     SIGNAL(resourcesReleasedByManager()),
                     SLOT(onResourceSetReleasedByManager()));
}

void AudioCallPolicyProxy::disconnectResourceSet()
{
    TRACE
    Q_D(AudioCallPolicyProxy);

    if(!d->resourceSet)
    {
        DEBUG_T("Resource set not configured.");
        return;
    }

    DEBUG_T("Disconnecting resource set signals.");
    QObject::disconnect(d->resourceSet,
                        SIGNAL(resourcesGranted(QList<ResourcePolicy::ResourceType>)),
                        this,
                        SLOT(onResourceSetGranted()));
    QObject::disconnect(d->resourceSet,
                        SIGNAL(resourcesDenied()),
                        this,
                        SLOT(onResourceSetDenied()));
    QObject::disconnect(d->resourceSet,
                        SIGNAL(lostResources()),
                        this,
                        SLOT(onResourceSetLost()));
    QObject::disconnect(d->resourceSet,
                        SIGNAL(errorCallback(quint32, const char*)),
                        this,
                        SLOT(onResourceSetError(quint32, const char*)));
    QObject::disconnect(d->resourceSet,
                        SIGNAL(resourcesBecameAvailable(QList<ResourcePolicy::ResourceType>)),
                        this,
                        SLOT(onResourceSetBecameAvailable(QList<ResourcePolicy::ResourceType>)));
    QObject::disconnect(d->resourceSet,
                        SIGNAL(resourcesReleased()),
                        this,
                        SLOT(onResourceSetReleased()));
    QObject::disconnect(d->resourceSet,
                        SIGNAL(resourcesReleasedByManager()),
                        this,
                        SLOT(onResourceSetReleasedByManager()));
}

void AudioCallPolicyProxy::onResourceSetError(quint32 errno, const char *errorMessage)
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    d->onAcquireReceiver = NULL;
    d->onAcquireMethod = QString::null;
    this->hangup();
}

void AudioCallPolicyProxy::onResourceSetGranted()
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    if(!d->onAcquireReceiver || d->onAcquireMethod.isNull() || d->onAcquireMethod.isEmpty())
    {
        DEBUG_T("No receiver or method to invoke.");
        return;
    }

    QTimer::singleShot(0, d->onAcquireReceiver, qPrintable(d->onAcquireMethod));
    d->onAcquireReceiver = NULL;
    d->onAcquireMethod = QString::null;
}

void AudioCallPolicyProxy::onResourceSetDenied()
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    d->onAcquireReceiver = NULL;
    d->onAcquireMethod = QString::null;
    this->hangup();
}

void AudioCallPolicyProxy::onResourceSetLost()
{
    TRACE
    Q_D(AudioCallPolicyProxy);
    d->onAcquireReceiver = NULL;
    d->onAcquireMethod = QString::null;
    this->hangup();
}

void AudioCallPolicyProxy::onResourceSetBecameAvailable(QList<ResourcePolicy::ResourceType> resources)
{
    TRACE
}

void AudioCallPolicyProxy::onResourceSetReleased()
{
    TRACE
}

void AudioCallPolicyProxy::onResourceSetReleasedByManager()
{
    TRACE
}
