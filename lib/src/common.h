/*
 * This file is a part of the Voice Call Manager Plugin project.
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
#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

#define WARNING_T(message) qWarning() << QString("VoiceCall W: %1: %2").arg(Q_FUNC_INFO).arg(message);

#ifndef WANT_TRACE
#   define TRACE
#   define DEBUG_T(message) if (false) { };
#else
#   define TRACE qDebug() << QString("VoiceCall T: %1:%2%").arg(Q_FUNC_INFO).arg(__LINE__);
#   define DEBUG_T(message) qDebug("%s", QString("VoiceCall D: %1: %2").arg(Q_FUNC_INFO).arg(message).toUtf8().constData());
#endif

#endif // COMMON_H
