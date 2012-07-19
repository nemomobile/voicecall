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

class VoiceCallManagerPrivate
{
public:
    VoiceCallManagerPrivate(VoiceCallManager *pInstance)
        : instance(pInstance), activeVoiceCall(NULL)
    {/* ... */}

    VoiceCallManager *instance;

    AbstractVoiceCallHandler *activeVoiceCall;

    QHash<QString, AbstractVoiceCallProvider*> providers;

    QString errorString;
};

VoiceCallManager::VoiceCallManager(QObject *parent)
    : VoiceCallManagerInterface(parent), d(new VoiceCallManagerPrivate(this))
{
    TRACE
}

VoiceCallManager::~VoiceCallManager()
{
    TRACE
    delete this->d;
}

QString VoiceCallManager::errorString() const
{
    TRACE
    return d->errorString;
}

void VoiceCallManager::setError(const QString &errorString)
{
    d->errorString = errorString;
    qDebug() << d->errorString;
    emit this->error(d->errorString);
}

AbstractVoiceCallHandler* VoiceCallManager::activeVoiceCall() const
{
    TRACE
    return d->activeVoiceCall;
}

QList<AbstractVoiceCallProvider*> VoiceCallManager::providers() const
{
    TRACE
    return d->providers.values();
}

void VoiceCallManager::appendProvider(AbstractVoiceCallProvider *provider)
{
    TRACE
    if(d->providers.contains(provider->providerId())) return;

    qDebug() << "VCM: Registering voice call provider:" << provider->providerId();
    QObject::connect(provider,
                     SIGNAL(voiceCallsChanged()),
                     SIGNAL(voiceCallsChanged()));
    QObject::connect(provider,
                     SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)),
                     SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
    QObject::connect(provider,
                     SIGNAL(voiceCallRemoved(QString)),
                     SLOT(onVoiceCallRemoved(QString)));
    QObject::connect(provider,
                     SIGNAL(incomingVoiceCall(AbstractVoiceCallHandler*)),
                     SIGNAL(incomingVoiceCall(AbstractVoiceCallHandler*)));

    d->providers.insert(provider->providerId(), provider);
    emit this->providersChanged();
    emit this->providerAdded(provider);
}

void VoiceCallManager::removeProvider(AbstractVoiceCallProvider *provider)
{
    TRACE
    if(!d->providers.contains(provider->providerId())) return;

    qDebug() << "VCM: Deregistering voice call provider:" << provider->providerId();
    QObject::disconnect(provider,
                        SIGNAL(voiceCallsChanged()),
                        this,
                        SIGNAL(voiceCallsChanged()));
    QObject::disconnect(provider,
                        SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)),
                        this,
                        SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
    QObject::disconnect(provider,
                        SIGNAL(voiceCallRemoved(QString)),
                        this,
                        SLOT(onVoiceCallRemoved(QString)));
    QObject::disconnect(provider,
                        SIGNAL(incomingVoiceCall(AbstractVoiceCallHandler*)),
                        this,
                        SIGNAL(incomingVoiceCall(AbstractVoiceCallHandler*)));

    d->providers.remove(provider->providerId());
    emit this->providersChanged();
    emit this->providerRemoved(provider->providerId());
}

int VoiceCallManager::voiceCallCount() const
{
    TRACE
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
    return QList<AbstractVoiceCallHandler*>();
}

bool VoiceCallManager::dial(const QString &providerId, const QString &msisdn)
{
    TRACE
    AbstractVoiceCallProvider *provider = d->providers.value(providerId);

    if(!provider)
    {
        this->setError(QString("*** Unable to find voice call provider with id: ") + providerId);
        return false;
    }

    if(!provider->dial(msisdn))
    {
        this->setError(provider->errorString());
        return false;
    }

    return true;
}

void VoiceCallManager::silenceNotifications()
{
    TRACE
    emit this->silenceRingtoneNotification();
}

void VoiceCallManager::onVoiceCallAdded(AbstractVoiceCallHandler *handler)
{
    TRACE
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
    emit this->voiceCallRemoved(handlerId);
    emit this->voiceCallsChanged();
    if(d->activeVoiceCall->handlerId() == handlerId)
    {
        d->activeVoiceCall = NULL;
        emit this->activeVoiceCallChanged();
    }
}
