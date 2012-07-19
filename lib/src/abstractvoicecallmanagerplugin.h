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
#ifndef ABSTRACTVOICECALLMANAGERPLUGIN_H
#define ABSTRACTVOICECALLMANAGERPLUGIN_H

#include <QObject>

class VoiceCallManagerInterface;

class AbstractVoiceCallManagerPlugin : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString pluginId READ pluginId)
    Q_PROPERTY(QString pluginVersion READ pluginVersion)

public:
    explicit AbstractVoiceCallManagerPlugin(QObject *parent = 0) : QObject(parent) {/*...*/}
    virtual ~AbstractVoiceCallManagerPlugin() {/*...*/}

    virtual QString pluginId() const = 0;
    virtual QString pluginVersion() const = 0;

public Q_SLOTS:
    virtual bool initialize() = 0;
    virtual bool configure(VoiceCallManagerInterface *manager) = 0;
    virtual bool start() = 0;
    virtual bool suspend() = 0;
    virtual bool resume() = 0;
    virtual void finalize() = 0;
};

Q_DECLARE_INTERFACE(AbstractVoiceCallManagerPlugin, "stage.rubyx.voicecall.ManagerPlugin/1.0")

#endif // ABSTRACTVOICECALLMANAGERPLUGIN_H
