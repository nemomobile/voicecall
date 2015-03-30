/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2011-2015  Tom Swindell <tom.swindell@jollamobile.com>
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
#include "callchannelhandler.h"

#include "farstreamchannel.h"
#include "telepathyprovider.h"

#include <TelepathyQt/Channel>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>

#include <TelepathyQt/CallChannel>
#include <TelepathyQt/CallContent>
#include <TelepathyQt/Farstream/Channel>

#include <QElapsedTimer>
#include <qmath.h>

static quint64 get_tick()
{
#if defined(CLOCK_BOOTTIME)
   int id = CLOCK_BOOTTIME;
#else
   int id = CLOCK_MONOTONIC;
#endif

  quint64 res = 0;

  struct timespec ts;

  if(clock_gettime(id, &ts) == 0) {
    res = ts.tv_sec;
    res *= 1000;
    res += ts.tv_nsec / 1000000;
  }

  return res;
}

class CallChannelHandlerPrivate
{
    Q_DECLARE_PUBLIC(CallChannelHandler)

public:
    CallChannelHandlerPrivate(CallChannelHandler *q, const QString &id, Tp::CallChannelPtr c, const QDateTime &s, TelepathyProvider *p)
        : q_ptr(q), handlerId(id), provider(p), startedAt(s), status(AbstractVoiceCallHandler::STATUS_NULL),
          channel(c), fsChannel(NULL), duration(0), durationTimerId(-1), isEmergency(false),
          isForwarded(false), isIncoming(false)
    { /* ... */ }

    CallChannelHandler  *q_ptr;

    QString            handlerId;
    TelepathyProvider *provider;

    QDateTime          startedAt;

    AbstractVoiceCallHandler::VoiceCallStatus status;

    Tp::CallChannelPtr channel; // CallChannel or StreamedMediaChannel
    FarstreamChannel *fsChannel;

    quint64 duration;
    quint64 connectedAt;
    int durationTimerId;
    QElapsedTimer elapsedTimer;
    bool isEmergency;
    bool isForwarded;
    bool isIncoming;
};

CallChannelHandler::CallChannelHandler(const QString &id, Tp::CallChannelPtr channel, const QDateTime &userActionTime, TelepathyProvider *provider)
    : AbstractVoiceCallHandler(provider), d_ptr(new CallChannelHandlerPrivate(this, id, channel, userActionTime, provider))
{
    TRACE
    Q_D(CallChannelHandler);

    QObject::connect(this, SIGNAL(statusChanged(VoiceCallStatus)), SLOT(onStatusChanged()));

    QObject::connect(d->channel->becomeReady(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onCallChannelChannelReady(Tp::PendingOperation*)));

    QObject::connect(d->channel.data(),
                     SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                     SLOT(onCallChannelChannelInvalidated(Tp::DBusProxy*,QString,QString)));

    emit this->startedAtChanged(startedAt());
}

CallChannelHandler::~CallChannelHandler()
{
    TRACE
    delete this->d_ptr;
}

AbstractVoiceCallProvider* CallChannelHandler::provider() const
{
    TRACE
    Q_D(const CallChannelHandler);
    return d->provider;
}

QString CallChannelHandler::handlerId() const
{
    TRACE
    Q_D(const CallChannelHandler);
    return d->handlerId;
}

QString CallChannelHandler::lineId() const
{
    TRACE
    Q_D(const CallChannelHandler);
    if(!d->channel->isReady()) return QString::null;
    return d->channel->targetId();
}

QDateTime CallChannelHandler::startedAt() const
{
    TRACE
    Q_D(const CallChannelHandler);
    return d->startedAt;
}

int CallChannelHandler::duration() const
{
    TRACE
    Q_D(const CallChannelHandler);
    return int(qRound(d->duration/1000.0));
}

bool CallChannelHandler::isIncoming() const
{
    TRACE
    Q_D(const CallChannelHandler);
    return d->isIncoming;
}

bool CallChannelHandler::isMultiparty() const
{
    TRACE
    Q_D(const CallChannelHandler);
    if(!d->channel->isReady()) return false;
    return d->channel->isConference();
}

bool CallChannelHandler::isEmergency() const
{
    TRACE
    Q_D(const CallChannelHandler);
    if(!d->channel->isReady()) return false;
    return d->isEmergency;
}

bool CallChannelHandler::isForwarded() const
{
    TRACE
    Q_D(const CallChannelHandler);
    if(!d->channel->isReady()) return false;
    return d->isForwarded;
}

AbstractVoiceCallHandler::VoiceCallStatus CallChannelHandler::status() const
{
    TRACE
    Q_D(const CallChannelHandler);
    return d->status;
}

void CallChannelHandler::answer()
{
    TRACE
    Q_D(CallChannelHandler);

    QObject::connect(d->channel.data()->accept(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onCallChannelAcceptCallFinished(Tp::PendingOperation*)));

    setStatus(STATUS_ACTIVE);
}

void CallChannelHandler::hangup()
{
    TRACE
    Q_D(CallChannelHandler);

    QObject::connect(d->channel.data()->hangup(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onCallChannelHangupCallFinished(Tp::PendingOperation*)));
}

void CallChannelHandler::hold(bool on)
{
    TRACE
    Q_D(CallChannelHandler);
    Tp::Client::ChannelInterfaceHoldInterface *holdIface = new Tp::Client::ChannelInterfaceHoldInterface(d->channel.data(), this);
    holdIface->RequestHold(on);
}

//FIXME: Don't know what telepathy API provides this.
void CallChannelHandler::deflect(const QString &target)
{
    TRACE
    Q_UNUSED(target)
    emit this->error("NOT IMPLEMENTED YET!");
}

void CallChannelHandler::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(CallChannelHandler);
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

    dtmfIface->StartTone(1, toneId, 0);
}

void CallChannelHandler::onCallChannelChannelReady(Tp::PendingOperation *op)
{
    TRACE
    Q_D(CallChannelHandler);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        return;
    }

    DEBUG_T("CallChannel Ready:");
    qDebug() << "\tType:" << d->channel->channelType();
    qDebug() << "\tInterfaces:" << d->channel->interfaces();

    DEBUG_T(QString("\tTransport: %1").arg(d->channel->initialTransportType()));
    DEBUG_T(QString("\tInitial Audio: %1").arg(d->channel->hasInitialAudio()));
    DEBUG_T(QString("\tAudio Name: %1").arg(d->channel->initialAudioName()));
    DEBUG_T(QString("\tInitial Video: %1").arg(d->channel->hasInitialVideo()));
    DEBUG_T(QString("\tVideo Name: %1").arg(d->channel->initialVideoName()));

    QObject::connect(d->channel.data(),
                     SIGNAL(contentAdded(Tp::CallContentPtr)),
                     SLOT(onCallChannelCallContentAdded(Tp::CallContentPtr)));
    QObject::connect(d->channel.data(),
                     SIGNAL(contentRemoved(Tp::CallContentPtr,Tp::CallStateReason)),
                     SLOT(onCallChannelCallContentRemoved(Tp::CallContentPtr,Tp::CallStateReason)));
    QObject::connect(d->channel.data(),
                     SIGNAL(callStateChanged(Tp::CallState)),
                     SLOT(onCallChannelCallStateChanged(Tp::CallState)));
    QObject::connect(d->channel.data(),
                     SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
                     SLOT(onCallChannelCallLocalHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)));

    if(d->channel->hasInitialAudio())
    {
        DEBUG_T("Processing channel initial content.");
        Tp::CallContentPtr audioContent = d->channel->contentByName(d->channel->initialAudioName());
        if(!audioContent || audioContent.isNull())
        {
            DEBUG_T("Audio content unavailable.");
        }
    }

    if(d->channel->handlerStreamingRequired())
    {
        DEBUG_T("Handler streaming is required, setting up farstream channels.");
        QObject::connect(Tp::Farstream::createChannel(d->channel),
                         SIGNAL(finished(Tp::PendingOperation*)),
                         SLOT(onFarstreamCreateChannelFinished(Tp::PendingOperation*)));
    }

    Tp::CallContents contents = d->channel->contents();
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

    d->channel->setRinging();

    emit lineIdChanged(lineId());
    emit multipartyChanged(isMultiparty());
    emit emergencyChanged(isEmergency());
    emit forwardedChanged(isForwarded());

    if(d->channel->isRequested())
    {
        setStatus(STATUS_ALERTING);
    }
    else
    {
        setStatus(STATUS_INCOMING);
    }

    d->isIncoming = !d->channel->isRequested();
}

void CallChannelHandler::onCallChannelChannelInvalidated(Tp::DBusProxy *, const QString &errorName, const QString &errorMessage)
{
    TRACE
    Q_D(CallChannelHandler);

    DEBUG_T(QString("Channel invalidated: ") + errorName + ": " + errorMessage);

    // It seems to get called twice.
    QObject::disconnect(d->channel.data(),
                        SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                        this,
                        SLOT(onCallChannelChannelInvalidated(Tp::DBusProxy*,QString,QString)));

    setStatus(STATUS_NULL);
    emit this->invalidated(errorName, errorMessage);
}

void CallChannelHandler::onCallChannelCallStateChanged(Tp::CallState state)
{
    TRACE

    switch(state)
    {
    case Tp::CallStateUnknown:
        setStatus(STATUS_NULL);
        break;

    case Tp::CallStatePendingInitiator:
        setStatus(STATUS_DIALING);
        break;

    case Tp::CallStateInitialising:
        setStatus(STATUS_DIALING);
        break;

    case Tp::CallStateInitialised:
        setStatus(STATUS_ALERTING);
        break;

    case Tp::CallStateAccepted:
        setStatus(STATUS_ALERTING);
        break;

    case Tp::CallStateActive:
        setStatus(STATUS_ACTIVE);
        break;

    case Tp::CallStateEnded:
        setStatus(STATUS_DISCONNECTED);
        this->invalidated(QString::null, QString::null);
        break;

    default:
        break;
    }
}

void CallChannelHandler::onCallChannelCallContentAdded(Tp::CallContentPtr content)
{
    TRACE
    Q_UNUSED(content)
}

void CallChannelHandler::onCallChannelCallContentRemoved(Tp::CallContentPtr content, Tp::CallStateReason reason)
{
    TRACE
    Q_UNUSED(content)
    Q_UNUSED(reason)
}

void CallChannelHandler::onCallChannelAcceptCallFinished(Tp::PendingOperation *op)
{
    TRACE
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        emit this->invalidated(op->errorName(), op->errorMessage());
        return;
    }

    setStatus(STATUS_ACTIVE);
}

void CallChannelHandler::onCallChannelHangupCallFinished(Tp::PendingOperation *op)
{
    TRACE
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        emit this->error(QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage()));
        emit this->invalidated(op->errorName(), op->errorMessage());
        return;
    }

    setStatus(STATUS_DISCONNECTED);
}

void CallChannelHandler::onFarstreamCreateChannelFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(CallChannelHandler);

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

void CallChannelHandler::timerEvent(QTimerEvent *event)
{
    TRACE
    Q_D(CallChannelHandler);

    if(isOngoing() && event->timerId() == d->durationTimerId)
    {
        d->duration = get_tick() - d->connectedAt;
        emit this->durationChanged(duration());
    }
}

void CallChannelHandler::onStatusChanged()
{
    TRACE
    Q_D(CallChannelHandler);

    if(isOngoing())
    {
        if (d->durationTimerId == -1) {
            d->durationTimerId = this->startTimer(1000);
            d->elapsedTimer.start();
            d->connectedAt = get_tick();
        }
    }
    else if (d->durationTimerId != -1)
    {
        this->killTimer(d->durationTimerId);
        d->durationTimerId = -1;
    }
}

void CallChannelHandler::setStatus(VoiceCallStatus newStatus)
{
    TRACE
    Q_D(CallChannelHandler);
    if (newStatus == d->status)
        return;

    d->status = newStatus;
    emit statusChanged(d->status);
}
