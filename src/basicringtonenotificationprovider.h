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
#ifndef BASICRINGTONENOTIFICATIONPROVIDER_H
#define BASICRINGTONENOTIFICATIONPROVIDER_H

#include <abstractvoicecallhandler.h>
#include <abstractvoicecallmanagerplugin.h>

class BasicRingtoneNotificationProvider : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

public:
    explicit BasicRingtoneNotificationProvider(QObject *parent = 0);
            ~BasicRingtoneNotificationProvider();

    QString pluginId() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void onVoiceCallAdded(AbstractVoiceCallHandler *handler);
    void onVoiceCallStatusChanged();
    void onMediaPlayerMediaStatusChanged();

private:
    class BasicRingtoneNotificationProviderPrivate *d_ptr;

    Q_DECLARE_PRIVATE(BasicRingtoneNotificationProvider)
};

#endif // BASICRINGTONENOTIFICATIONPROVIDER_H
