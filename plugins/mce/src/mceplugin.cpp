/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2015 Jolla Ltd.
 *
 * Contact: Tom Swindell <t.swindell@jollamobile.com>
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
#include "mceplugin.h"

#include <voicecallmanagerinterface.h>

#include <QtDBus>
#include <QtPlugin>

#define MCE_SERVICE "com.nokia.mce"
#define MCE_PATH    "/com/nokia/mce/request"
#define MCE_IFACE   "com.nokia.mce.request"

class McePluginPrivate
{
    Q_DECLARE_PUBLIC(McePlugin)

public:
    McePluginPrivate(McePlugin *q)
        : q_ptr(q),
          manager(NULL)
    { /* ... */ }

    McePlugin *q_ptr;

    VoiceCallManagerInterface *manager;

    QHash<QString, AbstractVoiceCallHandler*> calls;
};

McePlugin::McePlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent),
      d_ptr(new McePluginPrivate(this))
{
    TRACE
}

McePlugin::~McePlugin()
{
    TRACE
    delete this->d_ptr;
}

QString McePlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

bool McePlugin::initialize()
{
    TRACE
    return true;
}

bool McePlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(McePlugin);

    d->manager = manager;
    return true;
}

bool McePlugin::start()
{
    TRACE
    Q_D(const McePlugin);

    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), SLOT(onVoiceCallsChanged()));
    this->onVoiceCallsChanged();

    return true;
}

bool McePlugin::suspend()
{
    TRACE
    return true;
}

bool McePlugin::resume()
{
    TRACE
    return true;
}

void McePlugin::finalize()
{
    TRACE
}

void McePlugin::onVoiceCallsChanged()
{
    TRACE
    Q_D(McePlugin);

    QString state = "none";
    bool    isEmergency = false;

    QDBusMessage message = QDBusMessage::createMethodCall(MCE_SERVICE, MCE_PATH, MCE_IFACE, "req_call_state_change");

    if(d->manager->voiceCallCount() == 0)
    {
        d->calls.clear();
    }
    else
    {
        QStringList handlerIds;

        // Go through call handlers and start processing status changes.
        foreach(AbstractVoiceCallHandler *call, d->manager->voiceCalls())
        {
            handlerIds.append(call->handlerId());

            if(!d->calls.contains(call->handlerId()))
            {
                DEBUG_T("Registering call handler from internal dictionary.");

                QObject::connect(call, SIGNAL(statusChanged(VoiceCallStatus)), SLOT(onVoiceCallsChanged()));
                d->calls.insert(call->handlerId(), call);
            }

            isEmergency |= call->isEmergency();

            switch(call->status())
            {
            case AbstractVoiceCallHandler::STATUS_NULL:
            case AbstractVoiceCallHandler::STATUS_DISCONNECTED:
                continue;

            case AbstractVoiceCallHandler::STATUS_INCOMING:
                DEBUG_T("RINGING");
                state = "ringing";
                continue;

            case AbstractVoiceCallHandler::STATUS_DIALING:
            case AbstractVoiceCallHandler::STATUS_ALERTING:
            case AbstractVoiceCallHandler::STATUS_ACTIVE:
            case AbstractVoiceCallHandler::STATUS_HELD:
            case AbstractVoiceCallHandler::STATUS_WAITING:
                DEBUG_T("ACTIVE");
                if(state != "ringing") state = "active";
                continue;
            }
        }

        // Check for, and remove, removed calls from our call dictionary.
        foreach(QString callId, d->calls.keys())
        {
            if(handlerIds.contains(callId)) continue;

            AbstractVoiceCallHandler *call = d->calls.value(callId);

            DEBUG_T("Deregistering call handler from internal dictionary.");

            QObject::disconnect(call, SIGNAL(statusChanged(VoiceCallStatus)), this, SLOT(onVoiceCallsChanged()));

            d->calls.remove(callId);
        }
    }

    DEBUG_T("STATE: " + state);

    message << state;
    message << (isEmergency ? "emergency" : "normal");

    QDBusConnection::systemBus().send(message);
}
