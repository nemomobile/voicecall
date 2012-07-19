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

#include <ofonomodem.h>
#include <ofonovoicecallmanager.h>

class OfonoVoiceCallProviderPrivate
{
public:
    OfonoVoiceCallProviderPrivate()
        : ofonoManager(NULL), ofonoModem(NULL)
    { /* ... */ }

    OfonoVoiceCallManager   *ofonoManager;
    OfonoModem              *ofonoModem;

    QHash<QString,OfonoVoiceCallHandler*> voiceCalls;

    QString errorString;
    void setError(const QString &errorString)
    {
        this->errorString = errorString;
        debugMessage(errorString);
    }

    void debugMessage(const QString &message)
    {
        qDebug() << QString("OfonoVoiceCallProvider(") + ofonoModem->path() + "): " + message;
    }
};

OfonoVoiceCallProvider::OfonoVoiceCallProvider(const QString &path, QObject *parent)
    : AbstractVoiceCallProvider(parent), d(new OfonoVoiceCallProviderPrivate)
{
    TRACE
    d->ofonoModem = new OfonoModem(OfonoModem::ManualSelect, path, this);
    d->ofonoManager = new OfonoVoiceCallManager(OfonoModem::ManualSelect, path, this);
\
    this->setPoweredAndOnline();

    QObject::connect(d->ofonoManager, SIGNAL(callAdded(QString)), SLOT(onCallAdded(QString)));
    QObject::connect(d->ofonoManager, SIGNAL(callRemoved(QString)), SLOT(onCallRemoved(QString)));

    foreach(QString call, d->ofonoManager->getCalls())
    {
        this->onCallAdded(call);
    }
}

OfonoVoiceCallProvider::~OfonoVoiceCallProvider()
{
    TRACE
    delete this->d;
}

QString OfonoVoiceCallProvider::providerId() const
{
    TRACE
    return QString("ofono-") + d->ofonoManager->modem()->path();
}

QString OfonoVoiceCallProvider::providerType() const
{
    TRACE
    return "cellular";
}

QList<AbstractVoiceCallHandler*> OfonoVoiceCallProvider::voiceCalls() const
{
    TRACE
    QList<AbstractVoiceCallHandler*> results;

    foreach(AbstractVoiceCallHandler* handler, d->voiceCalls.values())
    {
        results.append(handler);
    }

    return results;
}

QString OfonoVoiceCallProvider::errorString() const
{
    TRACE
    return d->errorString;
}

bool OfonoVoiceCallProvider::dial(const QString &msisdn)
{
    TRACE
    if(!d->ofonoManager->isValid())
    {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->dial(msisdn, "default");
    return true;
}

OfonoModem* OfonoVoiceCallProvider::modem() const
{
    TRACE
    return d->ofonoModem;
}

bool OfonoVoiceCallProvider::setPoweredAndOnline(bool on)
{
    TRACE
    if(!d->ofonoModem || !d->ofonoModem->isValid()) return false;

    if(on)
    {
        if(!d->ofonoModem->powered())
        {
            d->debugMessage("Powering on modem");
            d->ofonoModem->setPowered(true);
        }
        else
        {
            d->debugMessage("Modem is powered");
        }

        if(!d->ofonoModem->online())
        {
            d->debugMessage("Setting modem to online");
            d->ofonoModem->setOnline(true);
        }
        else
        {
            d->debugMessage("Modem is online");
        }
    }
    else
    {
        if(d->ofonoModem->online())
        {
            d->debugMessage("Setting modem to offline");
            d->ofonoModem->setOnline(false);
        }
        else
        {
            d->debugMessage("Modem is offline");
        }

        if(d->ofonoModem->powered())
        {
            d->debugMessage("Powering down modem");
            d->ofonoModem->setPowered(false);
        }
        else
        {
            d->debugMessage("Modem is powered off");
        }
    }

    return true;
}

void OfonoVoiceCallProvider::onDialComplete(const bool status)
{
    TRACE
    if(!status)
    {
        d->setError(d->ofonoManager->errorMessage());
        return;
    }
}

void OfonoVoiceCallProvider::onCallAdded(const QString &call)
{
    TRACE
    if(d->voiceCalls.contains(call)) return;

    OfonoVoiceCallHandler *handler = new OfonoVoiceCallHandler(call, this);
    d->voiceCalls.insert(call, handler);

    qDebug() << "Call Added:";
    qDebug() << handler->lineId();
    qDebug() << handler->statusString();

    if(handler->status() == OfonoVoiceCallHandler::STATUS_INCOMING)
    {
        emit this->incomingVoiceCall(handler);
    }

    emit this->voiceCallAdded(handler);
    emit this->voiceCallsChanged();
}

void OfonoVoiceCallProvider::onCallRemoved(const QString &call)
{
    TRACE
    if(!d->voiceCalls.contains(call)) return;

    OfonoVoiceCallHandler *handler = d->voiceCalls.value(call);
    QString handlerId = handler->handlerId();
    d->voiceCalls.remove(call);
    handler->deleteLater();

    emit this->voiceCallRemoved(handlerId);
    emit this->voiceCallsChanged();
}
