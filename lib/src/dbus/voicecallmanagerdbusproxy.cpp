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
#include "voicecallmanagerdbusproxy.h"

#include <QDBusInterface>

class VoiceCallManagerDBusProxyPrivate
{
public:
    VoiceCallManagerDBusProxyPrivate()
        : manager(NULL), activeVoiceCall(NULL), connected(false)
    {/*...*/}

    QDBusInterface *manager;
    QVariantList    providers;

    VoiceCallHandlerDBusProxy *activeVoiceCall;

    bool connected;
};

VoiceCallManagerDBusProxy::VoiceCallManagerDBusProxy(QObject *parent)
    : QObject(parent), d(new VoiceCallManagerDBusProxyPrivate)
{
    TRACE
    d->manager = new QDBusInterface("stage.rubyx.voicecall",
                                    "/",
                                    "stage.rubyx.voicecall.VoiceCallManager",
                                    QDBusConnection::sessionBus(), this);

    if(d->manager->isValid())
    {
        this->initialize();
    }
}

VoiceCallManagerDBusProxy::~VoiceCallManagerDBusProxy()
{
    TRACE
    delete this->d;
}

void VoiceCallManagerDBusProxy::initialize()
{
    TRACE
    QObject::connect(d->manager, SIGNAL(error(QString)), SIGNAL(error(QString)));
    QObject::connect(d->manager, SIGNAL(providersChanged()), SIGNAL(providersChanged()));
    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), SIGNAL(voiceCallsChanged()));
    QObject::connect(d->manager, SIGNAL(activeVoiceCallChanged()), SLOT(onActiveVoiceCallChanged()));
    QObject::connect(d->manager, SIGNAL(incomingVoiceCall(QString,QString)), SIGNAL(incomingVoiceCall(QString,QString)));
    d->connected = true;
}

QStringList VoiceCallManagerDBusProxy::providers()
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    return d->manager->property("providers").toStringList();
}

QStringList VoiceCallManagerDBusProxy::voiceCalls()
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    return d->manager->property("voiceCalls").toStringList();
}

VoiceCallHandlerDBusProxy* VoiceCallManagerDBusProxy::activeVoiceCall()
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    return d->activeVoiceCall;
}

void VoiceCallManagerDBusProxy::dial(const QString &provider, const QString &msisdn)
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    d->manager->call("dial", provider, msisdn);
}

void VoiceCallManagerDBusProxy::silenceNotifications()
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    d->manager->call("silenceNotifications");
}

void VoiceCallManagerDBusProxy::startDtmfTone(const QString &tone, int volume)
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    d->manager->call("startDtmfTone", tone, volume);
}

void VoiceCallManagerDBusProxy::stopDtmfTone()
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    d->manager->call("stopDtmfTone");
}

void VoiceCallManagerDBusProxy::onActiveVoiceCallChanged()
{
    TRACE
    if(!d->connected && d->manager->isValid()) this->initialize();
    QString handlerId = d->manager->property("activeVoiceCall").toString();
    if(handlerId.isEmpty() && d->activeVoiceCall)
    {
        d->activeVoiceCall->deleteLater();
        d->activeVoiceCall = NULL;
    }
    else
    {
        qDebug() << handlerId;
        d->activeVoiceCall = new VoiceCallHandlerDBusProxy("/calls/active", this);
    }

    emit this->activeVoiceCallChanged();
}
