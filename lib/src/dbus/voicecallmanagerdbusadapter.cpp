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
#include "voicecallmanagerdbusadapter.h"

#include "voicecallmanagerinterface.h"

class VoiceCallManagerDBusAdapterPrivate
{
public:
    VoiceCallManagerDBusAdapterPrivate()
        : manager(NULL)
    {/*...*/}

    VoiceCallManagerInterface *manager;
};

VoiceCallManagerDBusAdapter::VoiceCallManagerDBusAdapter(QObject *parent)
    : QDBusAbstractAdaptor(parent), d(new VoiceCallManagerDBusAdapterPrivate)
{
    TRACE
}

VoiceCallManagerDBusAdapter::~VoiceCallManagerDBusAdapter()
{
    TRACE
    delete this->d;
}

void VoiceCallManagerDBusAdapter::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    d->manager = manager;
    QObject::connect(d->manager, SIGNAL(error(QString)), SIGNAL(error(QString)));
    QObject::connect(d->manager, SIGNAL(providersChanged()), SIGNAL(providersChanged()));
    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), SIGNAL(voiceCallsChanged()));
    QObject::connect(d->manager, SIGNAL(activeVoiceCallChanged()), SIGNAL(activeVoiceCallChanged()));
    QObject::connect(d->manager, SIGNAL(muteMicrophoneChanged()), SIGNAL(muteMicrophoneChanged()));
    QObject::connect(d->manager, SIGNAL(muteRingtoneChanged()), SIGNAL(muteRingtoneChanged()));
}

QStringList VoiceCallManagerDBusAdapter::providers() const
{
    TRACE
    QStringList results;

    foreach(AbstractVoiceCallProvider *provider, d->manager->providers())
    {
        results.append(provider->providerId() + ":" + provider->providerType());
    }

    return results;
}

QStringList VoiceCallManagerDBusAdapter::voiceCalls() const
{
    TRACE
    QStringList results;

    foreach(AbstractVoiceCallHandler *handler, d->manager->voiceCalls())
    {
        results.append(handler->handlerId());
    }

    return results;
}

QString VoiceCallManagerDBusAdapter::activeVoiceCall() const
{
    TRACE
    if(d->manager->activeVoiceCall())
    {
        return d->manager->activeVoiceCall()->handlerId();
    }
    return QString::null;
}

bool VoiceCallManagerDBusAdapter::muteMicrophone() const
{
    TRACE
    return d->manager->muteMicrophone();
}

bool VoiceCallManagerDBusAdapter::muteRingtone() const
{
    TRACE
    return d->manager->muteRingtone();
}

bool VoiceCallManagerDBusAdapter::setMuteMicrophone(bool on)
{
    TRACE
    d->manager->setMuteMicrophone(on);
    return true;
}

bool VoiceCallManagerDBusAdapter::setMuteRingtone(bool on)
{
    TRACE
    d->manager->setMuteRingtone(on);
    return true;
}

bool VoiceCallManagerDBusAdapter::dial(const QString &provider, const QString &msisdn)
{
    TRACE

    if(!d->manager->dial(provider, msisdn))
    {
        emit this->error(d->manager->errorString());
        return false;
    }

    return true;
}

bool VoiceCallManagerDBusAdapter::startDtmfTone(const QString &tone)
{
    TRACE
    d->manager->startDtmfTone(tone, 100);
    return true;
}

bool VoiceCallManagerDBusAdapter::stopDtmfTone()
{
    TRACE
    d->manager->stopDtmfTone();
    return true;
}
