/*
* This file is a part of the Voice Call Manager project
*
* Copyright (C) 2013 Jolla Ltd. <robin.burchell@jollamobile.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*/

#include <CommHistory/Event>
#include <CommHistory/EventModel>

#include "common.h"
#include "commhistoryplugin.h"

#include <voicecallmanagerinterface.h>

class CommHistoryPluginPrivate
{
    Q_DECLARE_PUBLIC(CommHistoryPlugin)

public:
    CommHistoryPluginPrivate(CommHistoryPlugin *q)
        : q_ptr(q), manager(NULL), currentCall(NULL)
    { /* ... */ }

    CommHistoryPlugin *q_ptr;
    VoiceCallManagerInterface *manager;

    AbstractVoiceCallHandler *currentCall;
};

CommHistoryPlugin::CommHistoryPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d_ptr(new CommHistoryPluginPrivate(this))
{
    TRACE
}

CommHistoryPlugin::~CommHistoryPlugin()
{
    TRACE
    delete this->d_ptr;
}

QString CommHistoryPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

bool CommHistoryPlugin::initialize()
{
    TRACE
    return true;
}

bool CommHistoryPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(CommHistoryPlugin);
    d->manager = manager;
    return true;
}

bool CommHistoryPlugin::start()
{
    TRACE
    Q_D(CommHistoryPlugin);

    return QObject::connect(d->manager, SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)), SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
}

bool CommHistoryPlugin::suspend()
{
    TRACE
    return true;
}

bool CommHistoryPlugin::resume()
{
    TRACE
    return true;
}

void CommHistoryPlugin::finalize()
{
    TRACE
}

void CommHistoryPlugin::onVoiceCallAdded(AbstractVoiceCallHandler *handler)
{
    TRACE
    Q_D(CommHistoryPlugin);

    QObject::connect(handler, SIGNAL(statusChanged()), SLOT(onVoiceCallStatusChanged()));
    d->currentCall = handler;
    onVoiceCallStatusChanged();
}

void CommHistoryPlugin::onVoiceCallStatusChanged()
{
    TRACE
    Q_D(CommHistoryPlugin);
    DEBUG_T(QString("Voice call status changed to: ") + d->currentCall->statusText());

    CommHistory::Event chEvent;
    QVariant boxedEvent = d->currentCall->property("commhistory_event");

    if (boxedEvent.isValid()) {
        chEvent = boxedEvent.value<CommHistory::Event>();
    }

    chEvent.setType(CommHistory::Event::CallEvent);
    chEvent.setStartTime(QDateTime::currentDateTime());
    chEvent.setEndTime(chEvent.startTime().addSecs(d->currentCall->duration()));
    chEvent.setRemoteUid(d->currentCall->lineId()); // TODO: correct?
    chEvent.setDirection(d->currentCall->isIncoming() ? CommHistory::Event::Inbound : CommHistory::Event::Outbound);
    chEvent.setIsEmergencyCall(d->currentCall->isEmergency()); // TODO: make sure these are correctly determined

    // hardcoded to cellular on the assumption that we will later on be using
    // telepathy again. TODO: this should be queried from providers, most
    // likely.
    chEvent.setLocalUid("/org/freedesktop/Telepathy/Account/ring/tel/account0""");

    if (d->currentCall->status() == AbstractVoiceCallHandler::STATUS_INCOMING) {
        // default missing
        chEvent.setIsMissedCall(true);
    } else if (d->currentCall->status() == AbstractVoiceCallHandler::STATUS_ACTIVE) {
        // and not missed once answered
        chEvent.setIsMissedCall(false);
    }

    CommHistory::EventModel eventModel;

    if (chEvent.isValid())
        eventModel.addEvent(chEvent);
    else
        eventModel.modifyEvent(chEvent);

    d->currentCall->setProperty("commhistory_event", QVariant::fromValue<CommHistory::Event>(chEvent));
}
