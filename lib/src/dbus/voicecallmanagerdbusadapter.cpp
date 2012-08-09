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

/*!
  \class VoiceCallManagerDBusAdapter
  \brief The D-Bus adapter for the voice call manager service.
*/
class VoiceCallManagerDBusAdapterPrivate
{
    Q_DECLARE_PUBLIC(VoiceCallManagerDBusAdapter)

public:
    VoiceCallManagerDBusAdapterPrivate(VoiceCallManagerDBusAdapter *q)
        : q_ptr(q), manager(NULL)
    {/*...*/}

    VoiceCallManagerDBusAdapter *q_ptr;
    VoiceCallManagerInterface *manager;
};

/*!
  Constructs a new DBus adapter.
*/
VoiceCallManagerDBusAdapter::VoiceCallManagerDBusAdapter(QObject *parent)
    : QDBusAbstractAdaptor(parent), d_ptr(new VoiceCallManagerDBusAdapterPrivate(this))
{
    TRACE
}

VoiceCallManagerDBusAdapter::~VoiceCallManagerDBusAdapter()
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    delete d;
}

/*!
  Configures the D-Bus adapter to work with the supplied manager. \a manager
*/
void VoiceCallManagerDBusAdapter::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager = manager;
    QObject::connect(d->manager, SIGNAL(error(QString)), SIGNAL(error(QString)));
    QObject::connect(d->manager, SIGNAL(providersChanged()), SIGNAL(providersChanged()));
    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), SIGNAL(voiceCallsChanged()));
    QObject::connect(d->manager, SIGNAL(activeVoiceCallChanged()), SIGNAL(activeVoiceCallChanged()));
    QObject::connect(d->manager, SIGNAL(audioModeChanged()), SIGNAL(audioModeChanged()));
    QObject::connect(d->manager, SIGNAL(audioRoutedChanged()), SIGNAL(audioRoutedChanged()));
    QObject::connect(d->manager, SIGNAL(microphoneMutedChanged()), SIGNAL(microphoneMutedChanged()));
    QObject::connect(d->manager, SIGNAL(speakerMutedChanged()), SIGNAL(speakerMutedChanged()));
}

/*!
  Returns a list of registered provider ids.
*/
QStringList VoiceCallManagerDBusAdapter::providers() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    QStringList results;

    foreach(AbstractVoiceCallProvider *provider, d->manager->providers())
    {
        results.append(provider->providerId() + ":" + provider->providerType());
    }

    return results;
}

/*!
  Returns a list of current voice call handler ids.
*/
QStringList VoiceCallManagerDBusAdapter::voiceCalls() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    QStringList results;

    foreach(AbstractVoiceCallHandler *handler, d->manager->voiceCalls())
    {
        results.append(handler->handlerId());
    }

    return results;
}

/*!
  Returns the currently active voice call handler id.
*/
QString VoiceCallManagerDBusAdapter::activeVoiceCall() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    if(d->manager->activeVoiceCall())
    {
        return d->manager->activeVoiceCall()->handlerId();
    }
    return QString::null;
}

/*!
  Returns the current audio routing policy name.
*/
QString VoiceCallManagerDBusAdapter::audioMode() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    return d->manager->audioMode();
}

bool VoiceCallManagerDBusAdapter::isAudioRouted() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    return d->manager->isAudioRouted();
}

/*!
  Returns the status of the microphone mute flag.

  /sa setMuteMicrophone(), muteRingtone()
*/
bool VoiceCallManagerDBusAdapter::isMicrophoneMuted() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    return d->manager->isMicrophoneMuted();
}

/*!
  Returns the status of the speaker mute flag.
*/
bool VoiceCallManagerDBusAdapter::isSpeakerMuted() const
{
    TRACE
    Q_D(const VoiceCallManagerDBusAdapter);
    return d->manager->isSpeakerMuted();
}

/*!
  Returns the status of the microphone mute flag.

  /sa setMuteMicrophone(), muteRingtone()
*/
bool VoiceCallManagerDBusAdapter::setAudioMode(const QString &mode)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager->setAudioMode(mode);
    return true;
}

bool VoiceCallManagerDBusAdapter::setAudioRouted(bool on)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager->setAudioRouted(on);
    return true;
}

/*!
  Sets the microphone mute flag to the value of on.

  \sa muteMicrophone()
*/
bool VoiceCallManagerDBusAdapter::setMuteMicrophone(bool on)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager->setMuteMicrophone(on);
    return true;
}

/*!
  Sets the ringtone mute flag to the value of on.
*/
bool VoiceCallManagerDBusAdapter::setMuteSpeaker(bool on)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager->setMuteSpeaker(on);
    return true;
}

/*!
  Initiates dialing a number using the provided providerId, and msisdn (phone number)
*/
bool VoiceCallManagerDBusAdapter::dial(const QString &provider, const QString &msisdn)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    if(!d->manager->dial(provider, msisdn))
    {
        emit this->error(d->manager->errorString());
        return false;
    }

    return true;
}

/*!
  Initiates sending of DTMF tones, where tone may be: 0-9, +, *, #, A-D.
*/
bool VoiceCallManagerDBusAdapter::startDtmfTone(const QString &tone)
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager->startDtmfTone(tone, 100);
    return true;
}

/*!
  Stops the sending of DTMF tones.
*/
bool VoiceCallManagerDBusAdapter::stopDtmfTone()
{
    TRACE
    Q_D(VoiceCallManagerDBusAdapter);
    d->manager->stopDtmfTone();
    return true;
}
