/*
 * This file is a part of the Voice Call Manager Plugin project.
 *
 * Copyright (C) 2011-2015 Jolla Ltd
 * Contact: Tom Swindell <tom.swindell@jollamobile.com>
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
#include "calleventhandler.h"

#include <abstractvoicecallprovider.h>

#include <CommHistory/Event>
#include <notification.h>

struct CallEventHandlerPrivate
{
    CallEventHandlerPrivate(AbstractVoiceCallHandler *pTarget, CommHistoryPlugin *pParent)
        : target(pTarget),
          parent(pParent),
          started(false),
          created(false),
          committed(false)
    {/*...*/}

    AbstractVoiceCallHandler *target;
    CommHistoryPlugin *parent;
    CommHistory::Event event;

    bool started;
    bool created;
    bool committed;
};

CallEventHandler::CallEventHandler(AbstractVoiceCallHandler *target, CommHistoryPlugin *parent)
    : QObject(parent), d_ptr(new CallEventHandlerPrivate(target, parent))
{
    TRACE
    Q_D(CallEventHandler);

    QObject::connect(parent->eventModel(), SIGNAL(eventsCommitted(QList<CommHistory::Event>, bool)),
                     this, SLOT(onEventsCommitted(QList<CommHistory::Event>, bool)));

    d->event.setType(CommHistory::Event::CallEvent);
    d->event.setStartTime(QDateTime::currentDateTime());
    d->event.setEndTime(d->event.startTime());
    d->event.setLocalUid(target->provider()->providerId());
    d->event.setRemoteUid(target->lineId().replace(QRegExp("^sip:"), ""));
    d->event.setDirection(target->isIncoming() ? CommHistory::Event::Inbound : CommHistory::Event::Outbound);
    d->event.setIsVideoCall(false);
    d->event.setIsEmergencyCall(target->isEmergency());

    pushEvent();

    QObject::connect(target, &AbstractVoiceCallHandler::statusChanged, this, &CallEventHandler::onCallStatusChanged);
    QObject::connect(target, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

CallEventHandler::~CallEventHandler()
{
    TRACE
}

void CallEventHandler::pushEvent()
{
    TRACE
    Q_D(CallEventHandler);

    if(d->created) {
        d->parent->eventModel()->modifyEvent(d->event);
    } else {
        d->created = d->parent->eventModel()->addEvent(d->event);
    }

    if(!d->created) WARNING_T("Failed to push event!");
}

void CallEventHandler::onEventsCommitted(QList<CommHistory::Event> events, bool success)
{
    TRACE
    Q_D(CallEventHandler);
    if(success) d->committed = true;
}

void CallEventHandler::onCallStatusChanged(AbstractVoiceCallHandler::VoiceCallStatus status)
{
    TRACE
    Q_D(CallEventHandler);
    bool modified = false;

    switch(status) {
    case AbstractVoiceCallHandler::STATUS_ACTIVE:
        d->started = true;

        d->event.setStartTime(d->target->startedAt());
        d->event.setEndTime(d->event.startTime());

        modified = true;
        break;

    case AbstractVoiceCallHandler::STATUS_INCOMING:
        d->event.setDirection(CommHistory::Event::Inbound);
        modified = true;
        break;

    case AbstractVoiceCallHandler::STATUS_NULL:
    case AbstractVoiceCallHandler::STATUS_DISCONNECTED:
        d->event.setEndTime(d->target->startedAt().addSecs(d->target->duration()));

        //TODO: We really need a "rejected()" signal so we can not display notifications
        // for rejected calls?
        if(!d->started) d->event.setIsMissedCall(true);

        modified = true;
        break;

    default:
        break;
    }

    if(modified) pushEvent();
}
