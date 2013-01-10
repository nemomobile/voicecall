/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "common.h"
#include "telepathyhandler.h"

#include "farstreamchannel.h"
#include "telepathyprovider.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>

#include <TelepathyQt/StreamedMediaChannel>

#include <TelepathyQt/CallChannel>
#include <TelepathyQt/CallContent>
#include <TelepathyQt/Farstream/Channel>

static const Tp::Features RequiredFeatures = Tp::Features() << Tp::StreamedMediaChannel::FeatureCore
                                                            << Tp::StreamedMediaChannel::FeatureLocalHoldState
                                                            << Tp::StreamedMediaChannel::FeatureStreams;

class TelepathyHandlerPrivate
{
    Q_DECLARE_PUBLIC(TelepathyHandler)

public:
    TelepathyHandlerPrivate(TelepathyHandler *q, const QString &id, Tp::ChannelPtr c, const QDateTime &s, TelepathyProvider *p)
        : q_ptr(q), handlerId(id), provider(p), startedAt(s), status(AbstractVoiceCallHandler::STATUS_NULL),
          channel(c), fsChannel(NULL), duration(0), durationTimerId(-1)
    { /* ... */ }

    TelepathyHandler  *q_ptr;

    QString            handlerId;
    TelepathyProvider *provider;

    QDateTime          startedAt;

    AbstractVoiceCallHandler::VoiceCallStatus status;

    Tp::ChannelPtr channel; // CallChannel or StreamedMediaChannel
    FarstreamChannel *fsChannel;

    int duration;
    int durationTimerId;
};

TelepathyHandler::TelepathyHandler(const QString &id, Tp::ChannelPtr channel, const QDateTime &userActionTime, TelepathyProvider *provider)
    : AbstractVoiceCallHandler(provider), d_ptr(new TelepathyHandlerPrivate(this, id, channel, userActionTime, provider))
{
    TRACE
    Q_D(const TelepathyHandler);

    QObject::connect(this, SIGNAL(statusChanged()), SLOT(onStatusChanged()));

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
    if(callChannel && !callChannel.isNull())
    {
        DEBUG_T("Found CallChannel interface.");
        Tp::Features features;
        features << Tp::CallChannel::FeatureCore;
        features << Tp::CallChannel::FeatureContents;

        QObject::connect(d->channel->becomeReady(features),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onCallChannelChannelReady(Tp::PendingOperation*)));

        QObject::connect(d->channel.data(),
                         SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                         SLOT(onCallChannelChannelInvalidated(Tp::DBusProxy*,QString,QString)));
    }

    //TODO: Remove when tp-ring is updated
    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(channel);
    if(streamChannel && !streamChannel.isNull())
    {
        DEBUG_T("Found StreamedMediaChannel interface.");
        Tp::Features features;
        features << Tp::StreamedMediaChannel::FeatureCore;
        features << Tp::StreamedMediaChannel::FeatureStreams;

        QObject::connect(d->channel->becomeReady(features),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onStreamedMediaChannelReady(Tp::PendingOperation*)));

        QObject::connect(d->channel.data(),
                         SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                         SLOT(onStreamedMediaChannelInvalidated(Tp::DBusProxy*,QString,QString)));
    }

    emit this->startedAtChanged();
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
    return d->duration;
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
        QObject::connect(callChannel.data()->accept(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onCallChannelAcceptCallFinished(Tp::PendingOperation*)));
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(d->channel);
    if(streamChannel && !streamChannel.isNull())
    {
        DEBUG_T("Found StreamedMediaChannel interface.");
        QObject::connect(streamChannel.data()->acceptCall(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onStreamedMediaChannelAcceptCallFinished(Tp::PendingOperation*)));
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
                         SLOT(onCallChannelHangupCallFinished(Tp::PendingOperation*)));
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(d->channel);
    if(streamChannel && !streamChannel.isNull())
    {
        DEBUG_T("Found StreamedMediaChannel interface.");
        QObject::connect(streamChannel.data()->hangupCall(),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onStreamedMediaChannelHangupCallFinished(Tp::PendingOperation*)));
    }

    d->status = STATUS_DISCONNECTED;
    emit statusChanged();
}

void TelepathyHandler::hold(bool on)
{
    TRACE
    Q_D(TelepathyHandler);
    Tp::Client::ChannelInterfaceHoldInterface *holdIface = new Tp::Client::ChannelInterfaceHoldInterface(d->channel.data(), this);
    holdIface->RequestHold(on);
}

//FIXME: Don't know what telepathy API provides this.
void TelepathyHandler::deflect(const QString &target)
{
    TRACE
    Q_UNUSED(target)
    emit this->error("NOT IMPLEMENTED YET!");
}

void TelepathyHandler::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(TelepathyHandler);
    Tp::Client::ChannelInterfaceDTMFInterface *dtmfIface = new Tp::Client::ChannelInterfaceDTMFInterface(d->channel.data(), this);

    bool ok = true;
    unsigned int toneId = tones.toInt(&ok);

    if(!ok)
    {
        if (tones == "*") toneId = 10;
        else if(tones == "#") toneId = 11;
        else if(tones == "A") toneId = 12;
        else if(tones == "B") toneId = 13;
        else if(tones == "C") toneId = 14;
        else if(tones == "D") toneId = 15;
        else return;
    }

    dtmfIface->StartTone(0, toneId, 50);
    //dtmfIface->MultipleTones(tones);
}

void TelepathyHandler::onCallChannelChannelReady(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(d->channel);
    DEBUG_T("CallChannel Ready:");
    qDebug() << "\tType:" << d->channel->channelType();
    qDebug() << "\tInterfaces:" << d->channel->interfaces();

    DEBUG_T(QString("\tTransport: %1").arg(callChannel->initialTransportType()));
    DEBUG_T(QString("\tInitial Audio: %1").arg(callChannel->hasInitialAudio()));
    DEBUG_T(QString("\tAudio Name: %1").arg(callChannel->initialAudioName()));
    DEBUG_T(QString("\tInitial Video: %1").arg(callChannel->hasInitialVideo()));
    DEBUG_T(QString("\tVideo Name: %1").arg(callChannel->initialVideoName()));

    QObject::connect(callChannel.data(),
                     SIGNAL(contentAdded(Tp::CallContentPtr)),
                     SLOT(onCallChannelCallContentAdded(Tp::CallContentPtr)));
    QObject::connect(callChannel.data(),
                     SIGNAL(contentRemoved(Tp::CallContentPtr,Tp::CallStateReason)),
                     SLOT(onCallChannelCallContentRemoved(Tp::CallContentPtr,Tp::CallStateReason)));
    QObject::connect(callChannel.data(),
                     SIGNAL(callStateChanged(Tp::CallState)),
                     SLOT(onCallChannelCallStateChanged(Tp::CallState)));
    QObject::connect(callChannel.data(),
                     SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
                     SLOT(onCallChannelCallLocalHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)));

    if(callChannel->hasInitialAudio())
    {
        DEBUG_T("Processing channel initial content.");
        Tp::CallContentPtr audioContent = callChannel->contentByName(callChannel->initialAudioName());
        if(!audioContent || audioContent.isNull())
        {
            DEBUG_T("Audio content unavailable.");
        }

    }

    if(callChannel->handlerStreamingRequired())
    {
        DEBUG_T("Handler streaming is required, setting up farstream channels.");
        QObject::connect(Tp::Farstream::createChannel(callChannel),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onFarstreamCreateChannelFinished(Tp::PendingOperation*)));
    }

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

    callChannel->setRinging();

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

void TelepathyHandler::onCallChannelChannelInvalidated(Tp::DBusProxy *, const QString &errorName, const QString &errorMessage)
{
    TRACE
    Q_D(TelepathyHandler);

    DEBUG_T(QString("Channel invalidated: ") + errorName + ": " + errorMessage);

    // It seems to get called twice.
    QObject::disconnect(d->channel.data(),
                        SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                        this,
                        SLOT(onCallChannelChannelInvalidated(Tp::DBusProxy*,QString,QString)));

    d->status = STATUS_NULL;
    emit this->statusChanged();
    emit this->invalidated(errorName, errorMessage);
}

void TelepathyHandler::onCallChannelCallStateChanged(Tp::CallState state)
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

void TelepathyHandler::onCallChannelCallContentAdded(Tp::CallContentPtr content)
{
    TRACE
    Q_UNUSED(content)
}

void TelepathyHandler::onCallChannelCallContentRemoved(Tp::CallContentPtr content, Tp::CallStateReason reason)
{
    TRACE
    Q_UNUSED(content)
    Q_UNUSED(reason)
}

void TelepathyHandler::onCallChannelAcceptCallFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        this->hangup();
        return;
    }

    d->status = STATUS_ACTIVE;
    emit this->statusChanged();
}

void TelepathyHandler::onCallChannelHangupCallFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        this->hangup();
        return;
    }

    d->status = STATUS_DISCONNECTED;
    emit this->statusChanged();

    emit this->invalidated("closed", "user");
}

void TelepathyHandler::onFarstreamCreateChannelFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        this->hangup();
        return;
    }

    Tp::Farstream::PendingChannel *pendingChannel = static_cast<Tp::Farstream::PendingChannel*>(op);
    if(!pendingChannel)
    {
        WARNING_T("Failed to cast pending channel.");
        this->hangup();
        return;
    }

    d->fsChannel = new FarstreamChannel(pendingChannel->tfChannel(), this);
    d->fsChannel->init();
}

void TelepathyHandler::onStreamedMediaChannelReady(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        return;
    }

    Tp::StreamedMediaChannelPtr streamChannel = Tp::StreamedMediaChannelPtr::dynamicCast(d->channel);
    DEBUG_T("StreamedMediaChannel Ready:");
    qDebug() << "\tType:" << d->channel->channelType();
    qDebug() << "\tInterfaces:" << d->channel->interfaces();

    if(streamChannel.data()->handlerStreamingRequired())
    {
        WARNING_T("NOT IMPLEMENTED - Handler streaming is required.");
    }

    QObject::connect(streamChannel.data(),
                     SIGNAL(streamAdded(Tp::StreamedMediaStreamPtr)),
                     SLOT(onStreamedMediaChannelStreamAdded(Tp::StreamedMediaStreamPtr)));

    QObject::connect(streamChannel.data(),
                     SIGNAL(streamRemoved(Tp::StreamedMediaStreamPtr)),
                     SLOT(onStreamedMediaChannelStreamRemoved(Tp::StreamedMediaStreamPtr)));

    QObject::connect(streamChannel.data(),
                     SIGNAL(streamError(Tp::StreamedMediaStreamPtr,Tp::MediaStreamError,QString)),
                     SLOT(onStreamedMediaChannelStreamError(Tp::StreamedMediaStreamPtr,Tp::MediaStreamError,QString)));

    QObject::connect(streamChannel.data(),
                     SIGNAL(streamStateChanged(Tp::StreamedMediaStreamPtr,Tp::MediaStreamState)),
                     SLOT(onStreamedMediaChannelStreamStateChanged(Tp::StreamedMediaStreamPtr,Tp::MediaStreamState)));

    if(d->channel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_GROUP))
    {
        DEBUG_T("Creating Group interface");
        Tp::Client::ChannelInterfaceGroupInterface *groupIface = new Tp::Client::ChannelInterfaceGroupInterface(d->channel.data(), this);
        QObject::connect(groupIface, SIGNAL(MembersChanged(QString,Tp::UIntList,Tp::UIntList,Tp::UIntList,Tp::UIntList,uint,uint)),
                         SLOT(onStreamedMediaChannelGroupMembersChanged(QString,Tp::UIntList,Tp::UIntList,Tp::UIntList,Tp::UIntList,uint,uint)));
    }

    if(d->channel->isRequested())
    {
        d->status = STATUS_DIALING;
    }
    else
    {
        d->status = STATUS_INCOMING;
    }

    emit statusChanged();
}

void TelepathyHandler::onStreamedMediaChannelInvalidated(Tp::DBusProxy *, const QString &errorName, const QString &errorMessage)
{
    TRACE
    Q_D(TelepathyHandler);
    DEBUG_T(QString("Channel invalidated: ") + errorName + ": " + errorMessage);

    // It seems to get called twice.
    QObject::disconnect(d->channel.data(),
                        SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                        this,
                        SLOT(onStreamedMediaChannelInvalidated(Tp::DBusProxy*,QString,QString)));

    d->status = STATUS_NULL;
    emit this->statusChanged();
    emit this->invalidated(errorName, errorMessage);
}

void TelepathyHandler::onStreamedMediaChannelStreamAdded(const Tp::StreamedMediaStreamPtr &stream)
{
    TRACE
    Q_UNUSED(stream)
}

void TelepathyHandler::onStreamedMediaChannelStreamRemoved(const Tp::StreamedMediaStreamPtr &stream)
{
    TRACE
    Q_UNUSED(stream)
}

void TelepathyHandler::onStreamedMediaChannelStreamError(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamError errorCode, const QString &errorMessage)
{
    TRACE
    Q_UNUSED(stream)
    Q_UNUSED(errorCode)

    emit this->error(QString("Telepathy Stream Error: %1").arg(errorMessage));
}

void TelepathyHandler::onStreamedMediaChannelStreamStateChanged(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamState state)
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
        d->status = STATUS_ALERTING;
        emit this->statusChanged();
        break;

    default:
        break;
    }
}

void TelepathyHandler::onStreamedMediaChannelAcceptCallFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        this->hangup();
        return;
    }

    d->status = STATUS_ACTIVE;
    emit this->statusChanged();
}

void TelepathyHandler::onStreamedMediaChannelHangupCallFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        this->hangup();
        return;
    }

    d->status = STATUS_DISCONNECTED;
    emit this->statusChanged();

    emit this->invalidated("closed", "user");
}

void TelepathyHandler::onStreamedMediaChannelGroupMembersChanged(QString message, Tp::UIntList added, Tp::UIntList removed, Tp::UIntList localPending, Tp::UIntList remotePending, uint actor, uint reason)
{
    TRACE
    Q_D(TelepathyHandler);

    Tp::Client::ChannelInterfaceGroupInterface *groupIface = new Tp::Client::ChannelInterfaceGroupInterface(d->channel.data(), this);

    QDBusPendingReply<Tp::UIntList> reply = groupIface->GetMembers();
    reply.waitForFinished();

    if(reply.isValid())
    {
        if(reply.value().count() == 0)
        {
            d->status = STATUS_DISCONNECTED;
        }
        else
        {
            d->status = STATUS_ACTIVE;
        }

        emit this->statusChanged();
    }
}

void TelepathyHandler::timerEvent(QTimerEvent *event)
{
    TRACE
    Q_D(TelepathyHandler);
    int status = this->status();

    if(event->timerId() == d->durationTimerId && (status == STATUS_ACTIVE || status == STATUS_HELD))
    {
        d->duration += 1;
        emit this->durationChanged();
    }
}

void TelepathyHandler::onStatusChanged()
{
    TRACE
    Q_D(TelepathyHandler);
    int status = this->status();

    if((status == STATUS_ACTIVE || status == STATUS_HELD) && d->durationTimerId == -1)
    {
        d->durationTimerId = this->startTimer(1000);
    }
    else
    {
        this->killTimer(d->durationTimerId);
        d->durationTimerId = -1;
    }
}
