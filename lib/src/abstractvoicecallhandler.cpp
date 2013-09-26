/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2013 Joona Petrell <joona.petrell@jollamobile.com>
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
#include "abstractvoicecallhandler.h"

QString AbstractVoiceCallHandler::statusText() const
{
    switch(status())
    {
        case STATUS_ACTIVE:
            return "active";
        case STATUS_HELD:
            return "held";
        case STATUS_DIALING:
            return "dialing";
        case STATUS_ALERTING:
            return "alerting";
        case STATUS_INCOMING:
            return "incoming";
        case STATUS_WAITING:
            return "waiting";
        case STATUS_DISCONNECTED:
            return "disconnected";

        default:
            return "null";
    }
}

bool AbstractVoiceCallHandler::isActive() const
{
    VoiceCallStatus status = this->status();
    return status == STATUS_ACTIVE;
}

bool AbstractVoiceCallHandler::isHeld() const
{
    VoiceCallStatus status = this->status();
    return status == STATUS_HELD;
}

bool AbstractVoiceCallHandler::isOngoing() const
{
    VoiceCallStatus status_ = status();
    return status_ == STATUS_ACTIVE || status_ == STATUS_HELD;
}
