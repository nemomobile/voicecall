/****************************************************************************
**
** Copyright (C) 2011-2012 Tom Swindell <t.swindell@rubyx.co.uk>
** All rights reserved.
**
** This file is part of the Voice Call UI project.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * The names of its contributors may NOT be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/
#include "common.h"
#include "voicecallhandlerdbusproxy.h"

#include <QDBusInterface>

class VoiceCallHandlerDBusProxyPrivate
{
public:
    VoiceCallHandlerDBusProxyPrivate()
    {/*...*/}

    QDBusInterface *interface;
};

VoiceCallHandlerDBusProxy::VoiceCallHandlerDBusProxy(const QString &path, QObject *parent)
    : QObject(parent), d(new VoiceCallHandlerDBusProxyPrivate)
{
    TRACE
    qDebug() << path;
    d->interface = new QDBusInterface("stage.rubyx.voicecall",
                                      path,
                                      "stage.rubyx.voicecall.VoiceCallHandler",
                                      QDBusConnection::sessionBus(), this);

    QObject::connect(d->interface, SIGNAL(statusChanged()), SIGNAL(statusChanged()));
    QObject::connect(d->interface, SIGNAL(msisdnChanged()), SIGNAL(msisdnChanged()));
    QObject::connect(d->interface, SIGNAL(emergencyChanged()), SIGNAL(emergencyChanged()));
    QObject::connect(d->interface, SIGNAL(multipartyChanged()), SIGNAL(multipartyChanged()));
}

VoiceCallHandlerDBusProxy::~VoiceCallHandlerDBusProxy()
{
    TRACE
    delete this->d;
}

QString VoiceCallHandlerDBusProxy::providerId() const
{
    TRACE
    return d->interface->property("providerId").toString();
}

QString VoiceCallHandlerDBusProxy::handlerId() const
{
    TRACE
    return d->interface->property("handlerId").toString();
}

QString VoiceCallHandlerDBusProxy::lineId() const
{
    TRACE
    return d->interface->property("lineId").toString();
}

QDateTime VoiceCallHandlerDBusProxy::startedAt() const
{
    TRACE
    return d->interface->property("startedAt").toDateTime();
}

bool VoiceCallHandlerDBusProxy::isMultiparty() const
{
    TRACE
    return d->interface->property("isMultiparty").toBool();
}

bool VoiceCallHandlerDBusProxy::isEmergency() const
{
    TRACE
    return d->interface->property("isEmergency").toBool();
}

int VoiceCallHandlerDBusProxy::status() const
{
    TRACE
    return d->interface->property("status").toInt();
}

void VoiceCallHandlerDBusProxy::answer()
{
    TRACE
    d->interface->call("answer");
}

void VoiceCallHandlerDBusProxy::hangup()
{
    TRACE
    d->interface->call("hangup");
}

void VoiceCallHandlerDBusProxy::deflect(const QString &target)
{
    TRACE
    d->interface->call("deflect", target);
}
