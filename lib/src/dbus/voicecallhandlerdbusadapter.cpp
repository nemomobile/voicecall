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
#include "voicecallhandlerdbusadapter.h"

#include "abstractvoicecallprovider.h"

class VoiceCallHandlerDBusAdapterPrivate
{
public:
    VoiceCallHandlerDBusAdapterPrivate(AbstractVoiceCallHandler *pHandler)
        : handler(pHandler)
    {/*...*/}

    AbstractVoiceCallHandler *handler;
};

VoiceCallHandlerDBusAdapter::VoiceCallHandlerDBusAdapter(AbstractVoiceCallHandler *handler)
    : QDBusAbstractAdaptor(handler), d(new VoiceCallHandlerDBusAdapterPrivate(handler))
{
    TRACE
    QObject::connect(d->handler, SIGNAL(statusChanged()), SIGNAL(statusChanged()));
    QObject::connect(d->handler, SIGNAL(lineIdChanged()), SIGNAL(lineIdChanged()));
    QObject::connect(d->handler, SIGNAL(emergencyChanged()), SIGNAL(emergencyChanged()));
    QObject::connect(d->handler, SIGNAL(multipartyChanged()), SIGNAL(multipartyChanged()));
}

VoiceCallHandlerDBusAdapter::~VoiceCallHandlerDBusAdapter()
{
    TRACE
    delete this->d;
}

QString VoiceCallHandlerDBusAdapter::providerId() const
{
    TRACE
    return d->handler->provider()->providerId();
}

QString VoiceCallHandlerDBusAdapter::handlerId() const
{
    TRACE
    return d->handler->handlerId();
}

QString VoiceCallHandlerDBusAdapter::lineId() const
{
    TRACE
    return d->handler->lineId();
}

QDateTime VoiceCallHandlerDBusAdapter::startedAt() const
{
    TRACE
    return d->handler->startedAt();
}

bool VoiceCallHandlerDBusAdapter::isMultiparty() const
{
    TRACE
    return d->handler->isMultiparty();
}

bool VoiceCallHandlerDBusAdapter::isEmergency() const
{
    TRACE
    return d->handler->isEmergency();
}

int VoiceCallHandlerDBusAdapter::status() const
{
    TRACE
    return (int)d->handler->status();
}

QString VoiceCallHandlerDBusAdapter::statusText() const
{
    TRACE
    return d->handler->statusText();
}

void VoiceCallHandlerDBusAdapter::answer()
{
    TRACE
    d->handler->answer();
}

void VoiceCallHandlerDBusAdapter::hangup()
{
    TRACE
    d->handler->hangup();
}

void VoiceCallHandlerDBusAdapter::deflect(const QString &target)
{
    TRACE
    d->handler->deflect(target);
}
