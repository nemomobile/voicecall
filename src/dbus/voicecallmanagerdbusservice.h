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
#ifndef VOICECALLMANAGERDBUSSERVICE_H
#define VOICECALLMANAGERDBUSSERVICE_H

#include <abstractvoicecallhandler.h>
#include <abstractvoicecallmanagerplugin.h>

class VoiceCallManagerDBusService : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

public:
    explicit VoiceCallManagerDBusService(QObject *parent = 0);
            ~VoiceCallManagerDBusService();

    QString pluginId() const;
    QString pluginVersion() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void onVoiceCallAdded(AbstractVoiceCallHandler *handler);
    void onVoiceCallRemoved(const QString &handlerId);

    void onActiveVoiceCallChanged();

private:
    class VoiceCallManagerDBusServicePrivate *d_ptr;

    Q_DECLARE_PRIVATE(VoiceCallManagerDBusService)
};

#endif // VOICECALLMANAGERDBUSSERVICE_H
