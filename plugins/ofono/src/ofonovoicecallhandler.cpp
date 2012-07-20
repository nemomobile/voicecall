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

#include <ofonovoicecall.h>

class OfonoVoiceCallHandlerPrivate
{
public:
    OfonoVoiceCallHandlerPrivate(OfonoVoiceCallProvider *pProvider)
        : provider(pProvider), ofonoVoiceCall(NULL)
    { /* ... */ }

    OfonoVoiceCallProvider *provider;

    OfonoVoiceCall *ofonoVoiceCall;
};

OfonoVoiceCallHandler::OfonoVoiceCallHandler(const QString &path, OfonoVoiceCallProvider *provider)
    : AbstractVoiceCallHandler(provider), d(new OfonoVoiceCallHandlerPrivate(provider))
{
    TRACE
    d->ofonoVoiceCall = new OfonoVoiceCall(path, this);

    QObject::connect(d->ofonoVoiceCall, SIGNAL(stateChanged(QString)), SIGNAL(statusChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(lineIdentificationChanged(QString)), SIGNAL(lineIdChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(emergencyChanged(bool)), SIGNAL(emergencyChanged()));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(multipartyChanged(bool)), SIGNAL(multipartyChanged()));
}

OfonoVoiceCallHandler::~OfonoVoiceCallHandler()
{
    TRACE
    delete this->d;
}

QString OfonoVoiceCallHandler::path() const
{
    TRACE
    return d->ofonoVoiceCall->path();
}

AbstractVoiceCallProvider* OfonoVoiceCallHandler::provider() const
{
    TRACE
    return d->provider;
}

QString OfonoVoiceCallHandler::handlerId() const
{
    TRACE
    return d->ofonoVoiceCall->path();
}

QString OfonoVoiceCallHandler::lineId() const
{
    TRACE
    return d->ofonoVoiceCall->lineIdentification();
}

QDateTime OfonoVoiceCallHandler::startedAt() const
{
    TRACE
    qDebug() << "CALL START TIME: " << d->ofonoVoiceCall->startTime();
    return QDateTime::fromString(d->ofonoVoiceCall->startTime(), "");
}

bool OfonoVoiceCallHandler::isMultiparty() const
{
    TRACE
    return d->ofonoVoiceCall->multiparty();
}

bool OfonoVoiceCallHandler::isEmergency() const
{
    TRACE
    return d->ofonoVoiceCall->emergency();
}

QString OfonoVoiceCallHandler::statusText() const
{
    TRACE
    return d->ofonoVoiceCall->state();
}

AbstractVoiceCallHandler::VoiceCallStatus OfonoVoiceCallHandler::status() const
{
    TRACE
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
    d->ofonoVoiceCall->answer();
}

void OfonoVoiceCallHandler::hangup()
{
    TRACE
    d->ofonoVoiceCall->hangup();
}

void OfonoVoiceCallHandler::deflect(const QString &target)
{
    TRACE
    d->ofonoVoiceCall->deflect(target);
}
