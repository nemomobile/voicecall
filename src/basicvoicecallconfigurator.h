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
#ifndef BASICVOICECALLCONFIGURATOR_H
#define BASICVOICECALLCONFIGURATOR_H

#include <QObject>

#include <voicecallmanagerinterface.h>
#include <abstractvoicecallmanagerplugin.h>

class BasicVoiceCallConfigurator : public QObject
{
    Q_OBJECT

public:
    explicit BasicVoiceCallConfigurator(QObject *parent = 0);
            ~BasicVoiceCallConfigurator();

Q_SIGNALS:

public Q_SLOTS:
    void configure(VoiceCallManagerInterface *manager);

protected Q_SLOTS:
    //TODO: Move to AbstractVoiceCallManagerPlugin
    bool installPlugin(AbstractVoiceCallManagerPlugin *plugin);
    void removePlugin(AbstractVoiceCallManagerPlugin *plugin);

private:
    class BasicVoiceCallConfiguratorPrivate *d_ptr;

    Q_DECLARE_PRIVATE(BasicVoiceCallConfigurator)
};

#endif // BASICVOICECALLCONFIGURATOR_H
