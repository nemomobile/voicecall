#include "common.h"
#include "telepathyhandler.h"

#include "telepathyprovider.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/PendingReady>

#include <TelepathyQt/ChannelInterface>

static const Tp::Features RequiredFeatures = Tp::Features() << Tp::StreamedMediaChannel::FeatureCore
                                                            << Tp::StreamedMediaChannel::FeatureLocalHoldState
                                                            << Tp::StreamedMediaChannel::FeatureStreams;

class TelepathyHandlerPrivate
{
    Q_DECLARE_PUBLIC(TelepathyHandler)

public:
    TelepathyHandlerPrivate(TelepathyHandler *q, const QString &id, Tp::StreamedMediaChannelPtr c, const QDateTime &s, TelepathyProvider *p)
        : q_ptr(q), handlerId(id), provider(p), startedAt(s), channel(c),
          status(AbstractVoiceCallHandler::STATUS_NULL),
          iCallState(NULL), iDtmf(NULL), iGroup(NULL), iHold(NULL), iServicePoint(NULL)
    { /* ... */ }

    TelepathyHandler  *q_ptr;

    QString            handlerId;
    TelepathyProvider *provider;

    QDateTime          startedAt;

    AbstractVoiceCallHandler::VoiceCallStatus status;

    // TODO: This should use "CallChannel", and, handle "Farstream" usage.
    //       Which relies on telepathy-ring implementing these ...

    // Streamed media channel and interfaces.
    Tp::StreamedMediaChannelPtr channel;
    Tp::Client::ChannelInterfaceCallStateInterface      *iCallState;
    Tp::Client::ChannelInterfaceDTMFInterface           *iDtmf;
    Tp::Client::ChannelInterfaceGroupInterface          *iGroup;
    Tp::Client::ChannelInterfaceHoldInterface           *iHold;
    Tp::Client::ChannelInterfaceServicePointInterface   *iServicePoint;

    void updateStatus(AbstractVoiceCallHandler::VoiceCallStatus status)
    {
        this->status = status;
        emit q_ptr->statusChanged();
    }
};

TelepathyHandler::TelepathyHandler(const QString &id, Tp::StreamedMediaChannelPtr channel, const QDateTime &userActionTime, TelepathyProvider *provider)
    : AbstractVoiceCallHandler(provider), d_ptr(new TelepathyHandlerPrivate(this, id, channel, userActionTime, provider))
{
    TRACE
    Q_D(const TelepathyHandler);

    QObject::connect(d->channel->becomeReady(RequiredFeatures),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onChannelReady(Tp::PendingOperation*)));

    QObject::connect(d->channel.data(),
                     SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                     SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));
}

TelepathyHandler::~TelepathyHandler()
{
    TRACE
    delete this->d_ptr;
}

AbstractVoiceCallProvider* TelepathyHandler::provider() const
{
    TRACE
    Q_D(const TelepathyHandler);
    return d->provider;
}

QString TelepathyHandler::handlerId() const
{
    TRACE
    Q_D(const TelepathyHandler);
    return d->handlerId;
}

QString TelepathyHandler::lineId() const
{
    TRACE
    Q_D(const TelepathyHandler);
    if(!d->channel->isReady()) return QString::null;
    return d->channel->targetId();
}

QDateTime TelepathyHandler::startedAt() const
{
    TRACE
    Q_D(const TelepathyHandler);
    return d->startedAt;
}

int TelepathyHandler::duration() const
{
    TRACE
    Q_D(const TelepathyHandler);
    return d->startedAt.secsTo(QDateTime());
}

bool TelepathyHandler::isMultiparty() const
{
    TRACE
    Q_D(const TelepathyHandler);
    if(!d->channel->isReady()) return false;
    return d->channel->isConference();
}

bool TelepathyHandler::isEmergency() const
{
    TRACE
    Q_D(const TelepathyHandler);
    if(!d->channel->isReady()) return false;
    return false;
}

AbstractVoiceCallHandler::VoiceCallStatus TelepathyHandler::status() const
{
    TRACE
    Q_D(const TelepathyHandler);
    return d->status;
}

//TODO: Move to AbstractVoiceCallHandler as this is generic.
QString TelepathyHandler::statusText() const
{
    TRACE
    Q_D(const TelepathyHandler);

    switch(d->status)
    {
    case STATUS_ACTIVE:
        return "active";
    case STATUS_HELD:
        return "held";
    case STATUS_DIALING:
        return "dialing";
    case STATUS_ALERTING:
        return "alerting";
    case STATUS_INCOMING:
        return "incoming";
    case STATUS_WAITING:
        return "waiting";
    case STATUS_DISCONNECTED:
        return "disconnected";

    default:
        return "null";
    }
}

void TelepathyHandler::answer()
{
    TRACE
    Q_D(TelepathyHandler);
    QObject::connect(d->channel->acceptCall(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onAcceptCallFinished(Tp::PendingOperation*)));
}

void TelepathyHandler::hangup()
{
    TRACE
    Q_D(TelepathyHandler);
    d->channel->hangupCall();
    QObject::connect(d->channel->hangupCall(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onHangupCallFinished(Tp::PendingOperation*)));
}

//FIXME: Don't know what telepathy API provides this.
void TelepathyHandler::deflect(const QString &target)
{
    TRACE
    Q_UNUSED(target)
    emit this->error("NOT IMPLEMENTED YET!");
}

void TelepathyHandler::onChannelReady(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        return;
    }

    DEBUG_T("Channel Ready:");
    qDebug() << "\tType:" << d->channel->channelType();
    qDebug() << "\tInterfaces:" << d->channel->interfaces();

    //TODO: Implement handle for Splittable (DRAFT) interface?
    if(d->channel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_CALL_STATE))
    {
        DEBUG_T("Constructing call state interface.");
        d->iCallState = new Tp::Client::ChannelInterfaceCallStateInterface(d->channel.data(), this);
        QObject::connect(d->iCallState,
                         SIGNAL(CallStateChanged(uint,uint)),
                         SLOT(onCallStateChanged(uint,uint)));
        //TODO: d->iCallState->GetCallStates();
    }
    if(d->channel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_DTMF))
    {
        DEBUG_T("Constructing dtmf interface.");
        d->iDtmf = new Tp::Client::ChannelInterfaceDTMFInterface(d->channel.data());
    }
    if(d->channel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_GROUP))
    {
        DEBUG_T("Constructing group interface.");
        d->iGroup = new Tp::Client::ChannelInterfaceGroupInterface(d->channel.data());
    }
    if(d->channel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_HOLD))
    {
        DEBUG_T("Constructing hold interface.");
        d->iHold = new Tp::Client::ChannelInterfaceHoldInterface(d->channel.data());
    }
    if(d->channel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_SERVICE_POINT))
    {
        DEBUG_T("Constructing service point interface.");
        d->iServicePoint = new Tp::Client::ChannelInterfaceServicePointInterface(d->channel.data());
    }

    QObject::connect(d->channel.data(),
                     SIGNAL(streamAdded(Tp::StreamedMediaStreamPtr)),
                     SLOT(onStreamAdded(Tp::StreamedMediaStreamPtr)));

    QObject::connect(d->channel.data(),
                     SIGNAL(streamRemoved(Tp::StreamedMediaStreamPtr)),
                     SLOT(onStreamRemoved(Tp::StreamedMediaStreamPtr)));

    QObject::connect(d->channel.data(),
                     SIGNAL(streamError(Tp::StreamedMediaStreamPtr,Tp::MediaStreamError,QString)),
                     SLOT(onStreamError(Tp::StreamedMediaStreamPtr,Tp::MediaStreamError,QString)));

    QObject::connect(d->channel.data(),
                     SIGNAL(streamStateChanged(Tp::StreamedMediaStreamPtr,Tp::MediaStreamState)),
                     SLOT(onStreamStateChanged(Tp::StreamedMediaStreamPtr,Tp::MediaStreamState)));

    //FIXME: Doesn't work, probably not the right way to do things anyway (interface access is better)
    QObject::connect(d->channel.data(), SIGNAL(propertyChanged(QString)), SLOT(onChannelPropertyChanged(QString)));


    if(d->channel->awaitingLocalAnswer())
    {
        d->status = STATUS_INCOMING;
    }
    else if(d->channel->awaitingRemoteAnswer())
    {
        d->status = STATUS_ALERTING;
    }

    if(d->channel->handlerStreamingRequired())
    {
        //TODO: Implement farsight support for SIP and Video calls.
    }

    emit this->statusChanged();
}

void TelepathyHandler::onChannelPropertyChanged(const QString &property)
{
    TRACE
    DEBUG_T(QString("Channel property changed: ") + property);
}

void TelepathyHandler::onChannelInvalidated(Tp::DBusProxy *, const QString &errorName, const QString &errorMessage)
{
    TRACE
    Q_D(TelepathyHandler);

    DEBUG_T(QString("Channel invalidated: ") + errorName + ": " + errorMessage);

    // It seems to get called twice.
    QObject::disconnect(d->channel.data(),
                        SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                        this,
                        SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));

    emit this->invalidated(errorName, errorMessage);
}

void TelepathyHandler::onStreamAdded(const Tp::StreamedMediaStreamPtr &stream)
{
    TRACE
    Q_UNUSED(stream)
}

void TelepathyHandler::onStreamRemoved(const Tp::StreamedMediaStreamPtr &stream)
{
    TRACE
    Q_UNUSED(stream)
}

void TelepathyHandler::onStreamError(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamError errorCode, const QString &errorMessage)
{
    TRACE
    Q_UNUSED(stream)
    Q_UNUSED(errorCode)

    emit this->error(QString("Telepathy Stream Error: %1").arg(errorMessage));
}

void TelepathyHandler::onStreamStateChanged(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamState state)
{
    TRACE
    Q_UNUSED(stream)
    Q_D(TelepathyHandler);

    switch(state)
    {
    case Tp::MediaStreamStateDisconnected:
        DEBUG_T("Media stream state disconnected.");
        d->status = STATUS_DISCONNECTED;
        emit this->statusChanged();
        break;

    case Tp::MediaStreamStateConnecting:
        DEBUG_T("Media stream state connecting.");
        break;

    case Tp::MediaStreamStateConnected:
        DEBUG_T("Media stream state connected.");
        d->status = STATUS_ACTIVE;
        emit this->statusChanged();
        break;

    default:
        break;
    }
}

void TelepathyHandler::onAcceptCallFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        return;
    }

    d->status = STATUS_ACTIVE;
    emit this->statusChanged();
}

void TelepathyHandler::onHangupCallFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        return;
    }

    d->status = STATUS_DISCONNECTED;
    emit this->statusChanged();

    emit this->invalidated("closed", "user");
}

void TelepathyHandler::onCallStateChanged(uint contact, uint state)
{
    TRACE
    DEBUG_T(QString("Call state changed for contact %1 to state %2").arg(contact).arg(state));
}
