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
#ifndef CALLEVENTHANDLER_H
#define CALLEVENTHANDLER_H

#include <QObject>

#include <abstractvoicecallhandler.h>
#include "commhistoryplugin.h"

namespace CommHistory {
    class Event;
}

class CallEventHandler : public QObject
{
    Q_OBJECT

public:
    explicit CallEventHandler(AbstractVoiceCallHandler *target, CommHistoryPlugin *parent);
            ~CallEventHandler();

protected Q_SLOTS:
    void onCallStatusChanged(AbstractVoiceCallHandler::VoiceCallStatus status);
    void onEventsCommitted(QList<CommHistory::Event> events, bool success);

protected:
    void pushEvent();

private:
    class CallEventHandlerPrivate *d_ptr;

    Q_DECLARE_PRIVATE(CallEventHandler)
};

#endif // CALLEVENTHANDLER_H
