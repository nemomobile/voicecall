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
#include "voicecallmanager.h"

#include <QHash>
#include <QUuid>

#include "audiocallpolicyproxy.h"

class VoiceCallManagerPrivate
{
    Q_DECLARE_PUBLIC(VoiceCallManager)

public:
    VoiceCallManagerPrivate(VoiceCallManager *q)
        : q_ptr(q), activeVoiceCall(NULL),
          audioMode("earpiece"), isAudioRouted(false), isMicrophoneMuted(false), isSpeakerMuted(false)
    {/* ... */}

    VoiceCallManager *q_ptr;

    QHash<QString, AbstractVoiceCallProvider*> providers;

    QHash<QString, AbstractVoiceCallHandler*> voiceCalls;

    AbstractVoiceCallHandler *activeVoiceCall;

    QString audioMode;
    bool isAudioRouted;
    bool isMicrophoneMuted;
    bool isSpeakerMuted;

    QString errorString;
};

VoiceCallManager::VoiceCallManager(QObject *parent)
    : VoiceCallManagerInterface(parent), d_ptr(new VoiceCallManagerPrivate(this))
{
    TRACE
}

VoiceCallManager::~VoiceCallManager()
{
    TRACE
    delete d_ptr;
}

QString VoiceCallManager::errorString() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->errorString;
}

void VoiceCallManager::setError(const QString &errorString)
{
    TRACE
    Q_D(VoiceCallManager);
    d->errorString = errorString;
    qDebug() << d->errorString;
    emit this->error(d->errorString);
}

AbstractVoiceCallHandler* VoiceCallManager::activeVoiceCall() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->activeVoiceCall;
}

QList<AbstractVoiceCallProvider*> VoiceCallManager::providers() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->providers.values();
}

void VoiceCallManager::appendProvider(AbstractVoiceCallProvider *provider)
{
    TRACE
    Q_D(VoiceCallManager);
    if(d->providers.contains(provider->providerId())) return;

    DEBUG_T(QString("VCM: Registering voice call provider: ") + provider->providerId());
    QObject::connect(provider,
                     SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)),
                     SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
    QObject::connect(provider,
                     SIGNAL(voiceCallRemoved(QString)),
                     SLOT(onVoiceCallRemoved(QString)));
    QObject::connect(provider,
                     SIGNAL(error(QString)),
                     SLOT(setError(QString)));

    d->providers.insert(provider->providerId(), provider);
    emit this->providersChanged();
    emit this->providerAdded(provider);
}

void VoiceCallManager::removeProvider(AbstractVoiceCallProvider *provider)
{
    TRACE
    Q_D(VoiceCallManager);
    if(!d->providers.contains(provider->providerId())) return;

    DEBUG_T(QString("VCM: Deregistering voice call provider: ") + provider->providerId());
    QObject::disconnect(provider,
                        SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)),
                        this,
                        SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
    QObject::disconnect(provider,
                        SIGNAL(voiceCallRemoved(QString)),
                        this,
                        SLOT(onVoiceCallRemoved(QString)));
    QObject::disconnect(provider,
                        SIGNAL(error(QString)), 
                        this,
                        SLOT(setError(QString)));

    d->providers.remove(provider->providerId());
    emit this->providersChanged();
    emit this->providerRemoved(provider->providerId());
}

QString VoiceCallManager::generateHandlerId()
{
    TRACE
    QString handlerId = QUuid::createUuid().toString().mid(1, 36); // Remove curly braces.
    return handlerId.replace('-', ""); // Remove dashes, (can't have dashes in dbus paths)
}

int VoiceCallManager::voiceCallCount() const
{
    TRACE
    Q_D(const VoiceCallManager);
    int result = 0;

    foreach(AbstractVoiceCallProvider *provider, d->providers.values())
    {
        result += provider->voiceCalls().length();
    }

    return result;
}

QList<AbstractVoiceCallHandler*> VoiceCallManager::voiceCalls() const
{
    TRACE
    Q_D(const VoiceCallManager);
    QList<AbstractVoiceCallHandler*> results;

    foreach(AbstractVoiceCallProvider *provider, d->providers)
    {
        results.append(provider->voiceCalls());
    }

    return results;
}

QString VoiceCallManager::audioMode() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->audioMode;
}

bool VoiceCallManager::isAudioRouted() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->isAudioRouted;
}

bool VoiceCallManager::isMicrophoneMuted() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->isMicrophoneMuted;
}

bool VoiceCallManager::isSpeakerMuted() const
{
    TRACE
    Q_D(const VoiceCallManager);
    return d->isSpeakerMuted;
}

void VoiceCallManager::setAudioMode(const QString &mode)
{
    TRACE
    Q_D(VoiceCallManager);
    d->audioMode = mode;
    emit this->setAudioModeRequested(mode);
    emit this->audioModeChanged();
}

void VoiceCallManager::setAudioRouted(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->isAudioRouted = on;
    emit this->setAudioRoutedRequested(on);
    emit this->audioRoutedChanged();
}

void VoiceCallManager::setMuteMicrophone(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->isMicrophoneMuted = on;
    emit this->setMuteMicrophoneRequested(on);
    emit this->microphoneMutedChanged();
}

void VoiceCallManager::setMuteSpeaker(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->isSpeakerMuted = on;
    emit this->setMuteSpeakerRequested(on);
    emit this->speakerMutedChanged();
}

void VoiceCallManager::onAudioModeChanged(const QString &mode)
{
    TRACE
    Q_D(VoiceCallManager);
    d->audioMode = mode;
    emit this->audioModeChanged();
}

void VoiceCallManager::onAudioRoutedChanged(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->isAudioRouted = on;
    emit this->audioRoutedChanged();
}

void VoiceCallManager::onMuteMicrophoneChanged(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->isMicrophoneMuted = on;
    emit this->microphoneMutedChanged();
}

void VoiceCallManager::onMuteSpeakerChanged(bool on)
{
    TRACE
    Q_D(VoiceCallManager);
    d->isSpeakerMuted = on;
    emit this->speakerMutedChanged();
}

bool VoiceCallManager::dial(const QString &providerId, const QString &msisdn)
{
    TRACE
    Q_D(VoiceCallManager);
    AbstractVoiceCallProvider *provider = d->providers.value(providerId);

    if(!provider)
    {
        this->setError(QString("*** Unable to find voice call provider with id: ") + providerId);
        return false;
    }

    return provider->dial(msisdn);
}

void VoiceCallManager::silenceRingtone()
{
    TRACE
    emit this->silenceRingtoneRequested();
}

void VoiceCallManager::startEventTone(ToneType type, int volume)
{
    TRACE
    emit this->startEventToneRequested(type, volume);
}

void VoiceCallManager::stopEventTone()
{
    TRACE
    emit this->stopEventToneRequested();
}

void VoiceCallManager::startDtmfTone(const QString &tone, int volume)
{
    TRACE
    emit this->startDtmfToneRequested(tone, volume);
}

void VoiceCallManager::stopDtmfTone()
{
    TRACE
    emit this->stopDtmfToneRequested();
}

void VoiceCallManager::onVoiceCallAdded(AbstractVoiceCallHandler *handler)
{
    TRACE
    Q_D(VoiceCallManager);

    //AudioCallPolicyProxy *pHandler = new AudioCallPolicyProxy(handler, this);
    d->voiceCalls.insert(handler->handlerId(), handler);

    emit this->voiceCallAdded(handler);
    emit this->voiceCallsChanged();

    if(!d->activeVoiceCall)
    {
        d->activeVoiceCall = handler;
        emit this->activeVoiceCallChanged();
    }
}

void VoiceCallManager::onVoiceCallRemoved(const QString &handlerId)
{
    TRACE
    Q_D(VoiceCallManager);

    AbstractVoiceCallHandler *handler = d->voiceCalls.value(handlerId);
    d->voiceCalls.remove(handlerId);

    emit this->voiceCallRemoved(handlerId);
    emit this->voiceCallsChanged();

    if (d->activeVoiceCall && d->activeVoiceCall->handlerId() == handlerId)
    {
        d->activeVoiceCall = NULL;
        emit this->activeVoiceCallChanged();
    }

    QSettings settings;
    settings.beginGroup(QLatin1String("Voice Counters"));

    // Update call time statistics
    if (handler->isIncoming()) {
        int received = settings.value(QLatin1String("Received")).toInt();
        received += handler->duration();
        DEBUG_T(QString::fromLatin1("Incoming call ended. Total incoming duration is now %1").arg(received));
        settings.setValue(QLatin1String("Received"), received);
        emit totalIncomingCallDurationChanged();
    } else {
        int dialled = settings.value(QLatin1String("Dialled")).toInt();
        dialled += handler->duration();
        DEBUG_T(QString::fromLatin1("Outgoing call ended. Total outgoing duration is now %1").arg(dialled));
        settings.setValue(QLatin1String("Dialled"), dialled);
        emit totalOutgoingCallDurationChanged();
    }

    settings.endGroup();

    handler->deleteLater();
}

int VoiceCallManager::totalOutgoingCallDuration() const
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Voice Counters"));
    int dialled = settings.value(QLatin1String("Dialled")).toInt();
    DEBUG_T(QString::fromLatin1("Request for outgoing call duration. Total: %1").arg(dialled));
    return dialled;
}

int VoiceCallManager::totalIncomingCallDuration() const
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Voice Counters"));
    int received = settings.value(QLatin1String("Received")).toInt();
    DEBUG_T(QString::fromLatin1("Request for incoming call duration. Total: %1").arg(received));
    return received;
}

void VoiceCallManager::resetCallDurationCounters()
{
    {
        QSettings settings;
        settings.beginGroup(QLatin1String("Voice Counters"));
        settings.setValue(QLatin1String("Received"), 0);
        settings.setValue(QLatin1String("Dialled"), 0);
        settings.endGroup();
    }

    emit totalOutgoingCallDurationChanged();
    emit totalIncomingCallDurationChanged();
}

