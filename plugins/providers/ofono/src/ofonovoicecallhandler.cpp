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

#include <qofonovoicecall.h>

#include <QTimerEvent>

class OfonoVoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(OfonoVoiceCallHandler)

public:
    OfonoVoiceCallHandlerPrivate(OfonoVoiceCallHandler *q, const QString &pHandlerId, OfonoVoiceCallProvider *pProvider)
        : q_ptr(q), handlerId(pHandlerId), provider(pProvider), ofonoVoiceCall(NULL), duration(0), durationTimerId(-1)
    { /* ... */ }

    OfonoVoiceCallHandler *q_ptr;

    QString handlerId;

    OfonoVoiceCallProvider *provider;

    QOfonoVoiceCall *ofonoVoiceCall;

    int duration;
    int durationTimerId;
};

OfonoVoiceCallHandler::OfonoVoiceCallHandler(const QString &handlerId, const QString &path, OfonoVoiceCallProvider *provider)
    : AbstractVoiceCallHandler(provider), d_ptr(new OfonoVoiceCallHandlerPrivate(this, handlerId, provider))
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall = new QOfonoVoiceCall(this);
    d->ofonoVoiceCall->setVoiceCallPath(path);

    QObject::connect(d->ofonoVoiceCall, SIGNAL(stateChanged(QString)), SIGNAL(statusChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(lineIdentificationChanged(QString)), SIGNAL(lineIdChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(emergencyChanged(bool)), SIGNAL(emergencyChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(multipartyChanged(bool)), SIGNAL(multipartyChanged()));

    QObject::connect(d->ofonoVoiceCall, SIGNAL(stateChanged(QString)), SLOT(onStatusChanged()));
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

//TODO: Fix me!
bool OfonoVoiceCallHandler::isIncoming() const
{
    TRACE
    //Q_D(const OfonoVoiceCallHandler);
    return false;
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

QString OfonoVoiceCallHandler::statusText() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->state();
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

void OfonoVoiceCallHandler::answer()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
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
    TRACE
    Q_D(OfonoVoiceCallHandler);
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
    Q_UNUSED(tones)
}

void OfonoVoiceCallHandler::timerEvent(QTimerEvent *event)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    int status = this->status();

    // Whilst call is active, increase duration by a second each second.
    if(event->timerId() == d->durationTimerId && (status == STATUS_ACTIVE || status == STATUS_HELD))
    {
        d->duration += 1;
        emit this->durationChanged();
    }
}

void OfonoVoiceCallHandler::onStatusChanged()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    int status = this->status();

    if((status == STATUS_ACTIVE || status == STATUS_HELD) && d->durationTimerId == -1)
    {
        d->durationTimerId = this->startTimer(1000);
    }
    else
    {
        this->killTimer(d->durationTimerId);
    }
}
