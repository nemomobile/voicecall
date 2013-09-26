/*
 * This file is a part of the Voice Call Manager Ofono Plugin project.
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "common.h"
#include "ofonovoicecallhandler.h"
#include "ofonovoicecallprovider.h"

#include <qofonovoicecallmanager.h>

#include <QTimerEvent>

class OfonoVoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(OfonoVoiceCallHandler)

public:
    OfonoVoiceCallHandlerPrivate(OfonoVoiceCallHandler *q, const QString &pHandlerId, OfonoVoiceCallProvider *pProvider, QOfonoVoiceCallManager *manager)
        : q_ptr(q), handlerId(pHandlerId), provider(pProvider), ofonoVoiceCallManager(manager), ofonoVoiceCall(NULL)
        , duration(0), durationTimerId(-1), isIncoming(false), forwarded(false)
    { /* ... */ }

    OfonoVoiceCallHandler *q_ptr;

    QString handlerId;

    OfonoVoiceCallProvider *provider;

    QOfonoVoiceCallManager *ofonoVoiceCallManager;
    QOfonoVoiceCall *ofonoVoiceCall;

    int duration;
    int durationTimerId;
    bool isIncoming;
    bool forwarded;

    QString disconnectReason;
};

OfonoVoiceCallHandler::OfonoVoiceCallHandler(const QString &handlerId, const QString &path, OfonoVoiceCallProvider *provider, QOfonoVoiceCallManager *manager)
    : AbstractVoiceCallHandler(provider), d_ptr(new OfonoVoiceCallHandlerPrivate(this, handlerId, provider, manager))
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall = new QOfonoVoiceCall(this);
    d->ofonoVoiceCall->setVoiceCallPath(path);
    d->isIncoming = d->ofonoVoiceCall->state() == QLatin1String("incoming");

    QObject::connect(d->ofonoVoiceCall, SIGNAL(stateChanged(QString)), SIGNAL(statusChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(lineIdentificationChanged(QString)), SIGNAL(lineIdChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(incomingLineChanged(QString)), SIGNAL(incomingLineIdChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(disconnectReason(QString)), SIGNAL(onDisconnectReason()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(emergencyChanged(bool)), SIGNAL(emergencyChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(multipartyChanged(bool)), SIGNAL(multipartyChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(remoteHeldChanged(bool)), SIGNAL(remoteHeldChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(remoteMultipartyChanged(bool)), SIGNAL(remoteMultipartyChanged()));

    QObject::connect(d->ofonoVoiceCall, SIGNAL(answerComplete(QOfonoVoiceCall::Error, QString)),
                                        SLOT(onAnswerComplete(QOfonoVoiceCall::Error, QString)));

    QObject::connect(d->ofonoVoiceCall, SIGNAL(hangupComplete(QOfonoVoiceCall::Error, QString)),
                                        SLOT(onHangupComplete(QOfonoVoiceCall::Error, QString)));

    QObject::connect(d->ofonoVoiceCall, SIGNAL(deflectComplete(QOfonoVoiceCall::Error, QString)),
                                        SLOT(onDeflectComplete(QOfonoVoiceCall::Error, QString)));

    QObject::connect(d->ofonoVoiceCallManager, SIGNAL(forwarded(QString)), SLOT(onForwarded(QString)));

    onStatusChanged();
}

OfonoVoiceCallHandler::~OfonoVoiceCallHandler()
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    delete d;
}

QString OfonoVoiceCallHandler::path() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->voiceCallPath();
}

AbstractVoiceCallProvider* OfonoVoiceCallHandler::provider() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->provider;
}

QString OfonoVoiceCallHandler::handlerId() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->handlerId;
}

QString OfonoVoiceCallHandler::lineId() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->lineIdentification();
}

QDateTime OfonoVoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    DEBUG_T(QString("CALL START TIME: ") + d->ofonoVoiceCall->startTime());
    return QDateTime::fromString(d->ofonoVoiceCall->startTime(), "");
}

int OfonoVoiceCallHandler::duration() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->duration;
}

bool OfonoVoiceCallHandler::isIncoming() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->isIncoming;
}

bool OfonoVoiceCallHandler::isMultiparty() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->multiparty();
}

bool OfonoVoiceCallHandler::isEmergency() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->emergency();
}

bool OfonoVoiceCallHandler::isForwarded() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->forwarded;
}

// XXX NOT IMPLEMENTED YET
bool OfonoVoiceCallHandler::isRemoteHeld() const
{
    TRACE
    return false;
}

// XXX NOT IMPLEMENTED YET
bool OfonoVoiceCallHandler::isRemoteMultiparty() const
{
    TRACE
    return false;
}

AbstractVoiceCallHandler::VoiceCallStatus OfonoVoiceCallHandler::status() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    QString state = d->ofonoVoiceCall->state();

    if(state == "active")
        return STATUS_ACTIVE;
    else if(state == "held")
        return STATUS_HELD;
    else if(state == "dialing")
        return STATUS_DIALING;
    else if(state == "alerting")
        return STATUS_ALERTING;
    else if(state == "incoming")
        return STATUS_INCOMING;
    else if(state == "waiting")
        return STATUS_WAITING;
    else if(state == "disconnected")
        return STATUS_DISCONNECTED;

    return STATUS_NULL;
}

QString OfonoVoiceCallHandler::disconnectReason() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->disconnectReason;
}

void OfonoVoiceCallHandler::answer()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    if (status() == STATUS_WAITING)
        d->ofonoVoiceCallManager->holdAndAnswer();
    else
        d->ofonoVoiceCall->answer();
}

void OfonoVoiceCallHandler::hangup()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall->hangup();
}

void OfonoVoiceCallHandler::hold(bool on)
{
    Q_UNUSED(on)
    TRACE
    Q_D(OfonoVoiceCallHandler);
    bool isHeld = status() == STATUS_HELD;
    if (isHeld == on)
        return;

    d->ofonoVoiceCallManager->swapCalls();
}

void OfonoVoiceCallHandler::deflect(const QString &target)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall->deflect(target);
}

void OfonoVoiceCallHandler::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCallManager->sendTones(tones);
}

void OfonoVoiceCallHandler::onAnswerComplete(QOfonoVoiceCall::Error error, const QString &errorString)
{
    TRACE
    if(error != QOfonoVoiceCall::NoError) emit this->error(errorString);
}

void OfonoVoiceCallHandler::onHangupComplete(QOfonoVoiceCall::Error error, const QString &errorString)
{
    TRACE
    if(error != QOfonoVoiceCall::NoError) emit this->error(errorString);
}

void OfonoVoiceCallHandler::onDeflectComplete(QOfonoVoiceCall::Error error, const QString &errorString)
{
    TRACE
    if(error != QOfonoVoiceCall::NoError) emit this->error(errorString);
}

void OfonoVoiceCallHandler::timerEvent(QTimerEvent *event)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);

    // Whilst call is active, increase duration by a second each second.
    if(isOngoing() && event->timerId() == d->durationTimerId)
    {
        d->duration += 1;
        emit this->durationChanged();
    }
}

void OfonoVoiceCallHandler::onStatusChanged()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);

    if(isOngoing() && d->durationTimerId == -1)
    {
        d->durationTimerId = this->startTimer(1000);
    }
    else if (d->durationTimerId != -1)
    {
        this->killTimer(d->durationTimerId);
        d->durationTimerId = -1;
    }
}

// XXX Experimental
void OfonoVoiceCallHandler::onForwarded(const QString &type)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    if(this->isActive()) {
        d->forwarded = true;
        emit this->forwardedChanged();
    }
}

void OfonoVoiceCallHandler::onDisconnectReason(const QString &reason)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->disconnectReason = reason;
    emit this->disconnectReasonChanged();
}
