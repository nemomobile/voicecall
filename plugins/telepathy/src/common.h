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
#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

#define DEBUG_T(message) qDebug() << QString("D:VCM: [%1] %2():%3: %4").arg(__FILE__).arg(__func__).arg(__LINE__).arg(message);
#define WARNING_T(message) qWarning() << QString("W:VCM: [%1] %2():%3: %4").arg(__FILE__).arg(__func__).arg(__LINE__).arg(message);

#ifndef WANT_TRACE
#   define TRACE
#else
#   define TRACE qDebug() << QString("T:VCM: [%1] %2():%3").arg(__FILE__).arg(__func__).arg(__LINE__);
#endif

#endif // COMMON_H
