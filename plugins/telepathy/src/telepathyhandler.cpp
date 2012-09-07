#include "common.h"
#include "telepathyhandler.h"

#include "telepathyprovider.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/StreamedMediaChannel>

#include <TelepathyQt/CallChannel>
#include <TelepathyQt/CallContent>

#include <TelepathyQt/PendingReady>

static const Tp::Features RequiredFeatures = Tp::Features() << Tp::StreamedMediaChannel::FeatureCore
                                                            << Tp::StreamedMediaChannel::FeatureLocalHoldState
                                                            << Tp::StreamedMediaChannel::FeatureStreams;

class TelepathyHandlerPrivate
{
    Q_DECLARE_PUBLIC(TelepathyHandler)

public:
    TelepathyHandlerPrivate(TelepathyHandler *q, const QString &id, Tp::ChannelPtr c, const QDateTime &s, TelepathyProvider *p)
        : q_ptr(q), handlerId(id), provider(p), startedAt(s), status(AbstractVoiceCallHandler::STATUS_NULL), channel(c),
          iCallState(NULL), iDtmf(NULL), iGroup(NULL), iHold(NULL), iServicePoint(NULL)
    { /* ... */ }

    TelepathyHandler  *q_ptr;

    QString            handlerId;
    TelepathyProvider *provider;

    QDateTime          startedAt;

    AbstractVoiceCallHandler::VoiceCallStatus status;

    //TODO: Implement two types of handler, for tpring and SIP.
    //      SIP can inherit from tpring handler but add farstream support.
    Tp::ChannelPtr channel; // CallChannel or StreamedMediaChannel

    // Streamed media channel and interfaces.
    Tp::Client::ChannelInterfaceCallStateInterface      *iCallState;
    Tp::Client::ChannelInterfaceDTMFInterface           *iDtmf;
    Tp::Client::ChannelInterfaceGroupInterface          *iGroup;
    Tp::Client::ChannelInterfaceHoldInterface           *iHold;
    Tp::Client::ChannelInterfaceServicePointInterface   *iServicePoint;
};

TelepathyHandler::TelepathyHandler(const QString &id, Tp::ChannelPtr channel, const QDateTime &userActionTime, TelepathyProvider *provider)
    : AbstractVoiceCallHandler(provider), d_ptr(new TelepathyHandlerPrivate(this, id, channel, userActionTime, provider))
{
    TRACE
    Q_D(const TelepathyHandler);

    Tp::Features features;

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
    if(callChannel && !callChannel.isNull())
    {
        features << Tp::CallChannel::FeatureCore;
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(channel);
    if(streamChannel && !streamChannel.isNull())
    {
        features << Tp::StreamedMediaChannel::FeatureCore;
    }

    QObject::connect(d->channel->becomeReady(features),
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

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(d->channel);
    if(callChannel && !callChannel.isNull())
    {
        DEBUG_T("Found CallChannel interface.");
        callChannel.data()->
        QObject::connect(callChannel.data()->accept(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onAcceptCallFinished(Tp::PendingOperation*)));


        Tp::CallContents contents = callChannel->contents();
        DEBUG_T(QString("number of contents: %1").arg(contents.size()));
        if (contents.size() > 0) {
            foreach (const Tp::CallContentPtr &content, contents) {
                Q_ASSERT(!content.isNull());
                DEBUG_T("Call Content");
                Tp::CallStreams streams = content->streams();
                foreach (const Tp::CallStreamPtr &stream, streams) {
                    DEBUG_T(QString("  Call stream: localSendingState=%1").arg(stream->localSendingState()));
                    DEBUG_T(QString("      members: %1").arg(stream.data()->remoteMembers().size()));
                    foreach(const Tp::ContactPtr contact, stream.data()->remoteMembers()) {
                        DEBUG_T(QString("        member %1").arg(contact->id()) + " remoteSendingState=" + stream->remoteSendingState(contact));
                    }
                    //onStreamAdded(stream);
                }
            }
        }
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(d->channel);
    if(streamChannel && !streamChannel.isNull())
    {
        DEBUG_T("Found CallChannel interface.");
        QObject::connect(streamChannel.data()->acceptCall(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onAcceptCallFinished(Tp::PendingOperation*)));
    }
    d->status = STATUS_ACTIVE;
    emit statusChanged();
}

void TelepathyHandler::hangup()
{
    TRACE
    Q_D(TelepathyHandler);

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(d->channel);
    if(callChannel && !callChannel.isNull())
    {
        DEBUG_T("Found CallChannel interface.");
        QObject::connect(callChannel.data()->hangup(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onHangupCallFinished(Tp::PendingOperation*)));
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(d->channel);
    if(streamChannel && !streamChannel.isNull())
    {
        DEBUG_T("Found CallChannel interface.");
        QObject::connect(streamChannel.data()->hangupCall(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onHangupCallFinished(Tp::PendingOperation*)));
    }
    d->status = STATUS_DISCONNECTED;
    emit statusChanged();
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

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(d->channel);
    if(callChannel && !callChannel.isNull())
    {
        DEBUG_T("Found CallChannel interface.");
        DEBUG_T(QString("\tTransport: %1").arg(callChannel->initialTransportType()));
        DEBUG_T(QString("\tInitial Audio: %1").arg(callChannel->hasInitialAudio()));
        DEBUG_T(QString("\tAudio Name: %1").arg(callChannel->initialAudioName()));
        DEBUG_T(QString("\tInitial Video: %1").arg(callChannel->hasInitialVideo()));
        DEBUG_T(QString("\tVideo Name: %1").arg(callChannel->initialVideoName()));

        QObject::connect(callChannel.data(),
                         SIGNAL(contentAdded(Tp::CallContentPtr)),
                         SLOT(onChannelCallContentAdded(Tp::CallContentPtr)));
        QObject::connect(callChannel.data(),
                         SIGNAL(contentRemoved(Tp::CallContentPtr,Tp::CallStateReason)),
                         SLOT(onChannelCallContentRemoved(Tp::CallContentPtr,Tp::CallStateReason)));
        QObject::connect(callChannel.data(),
                         SIGNAL(callStateChanged(Tp::CallState)),
                         SLOT(onChannelCallStateChanged(Tp::CallState)));
        QObject::connect(callChannel.data(),
                         SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
                         SLOT(onChannelCallLocalHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)));

        Tp::CallContents contents = callChannel->contents();
        DEBUG_T(QString("number of contents: %1").arg(contents.size()));
        if (contents.size() > 0) {
            foreach (const Tp::CallContentPtr &content, contents) {
                Q_ASSERT(!content.isNull());
                DEBUG_T("Call Content");
                Tp::CallStreams streams = content->streams();
                foreach (const Tp::CallStreamPtr &stream, streams) {
                    DEBUG_T(QString("  Call stream: localSendingState=%1").arg(stream->localSendingState()));
                    DEBUG_T(QString("      members: %1").arg(stream.data()->remoteMembers().size()));
                    foreach(const Tp::ContactPtr contact, stream.data()->remoteMembers()) {
                        DEBUG_T(QString("        member %1").arg(contact->id()) + " remoteSendingState=" + stream->remoteSendingState(contact));
                    }
                    //onStreamAdded(stream);
                }
            }
        }

        callChannel.data()->setRinging();
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(d->channel);
    if(streamChannel && !streamChannel.isNull())
    {
        if(streamChannel.data()->handlerStreamingRequired())
        {
            WARNING_T("NOT IMPLEMENTED YET - Handler streaming is required!");
            // setup telepathy farsight
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
    }

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

    //FIXME: Doesn't work, probably not the right way to do things anyway (interface access is better)
    QObject::connect(d->channel.data(), SIGNAL(propertyChanged(QString)), SLOT(onChannelPropertyChanged(QString)));

    if(d->channel->isRequested())
    {
        d->status = STATUS_ALERTING;
    }
    else
    {
        d->status = STATUS_INCOMING;
    }

    emit statusChanged();
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

    d->status = STATUS_NULL;
    emit this->statusChanged();
    emit this->invalidated(errorName, errorMessage);
}

void TelepathyHandler::onChannelCallStateChanged(Tp::CallState state)
{
    TRACE
    Q_D(TelepathyHandler);

    switch(state)
    {
    case Tp::CallStateUnknown:
        d->status = STATUS_NULL;
        break;

    case Tp::CallStatePendingInitiator:
        d->status = STATUS_DIALING;
        break;

    case Tp::CallStateInitialising:
        d->status = STATUS_DIALING;
        break;

    case Tp::CallStateInitialised:
        d->status = STATUS_ALERTING;
        break;

    case Tp::CallStateAccepted:
        d->status = STATUS_ALERTING;
        break;

    case Tp::CallStateActive:
        d->status = STATUS_ACTIVE;
        break;

    case Tp::CallStateEnded:
        d->status = STATUS_DISCONNECTED;
        break;

    default:
        break;
    }

    emit statusChanged();
}

void TelepathyHandler::onChannelCallContentAdded(Tp::CallContentPtr content)
{
    TRACE
    Q_UNUSED(content)
}

void TelepathyHandler::onChannelCallContentRemoved(Tp::CallContentPtr content, Tp::CallStateReason reason)
{
    TRACE
    Q_UNUSED(content)
    Q_UNUSED(reason)
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
