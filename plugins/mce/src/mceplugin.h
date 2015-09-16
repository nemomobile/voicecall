/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2015 Jolla Ltd.
 *
 * Contact: Tom Swindell <t.swindell@jollamobile.com>
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

#ifndef MCEPLUGIN_H
#define MCEPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>
#include <abstractvoicecallhandler.h>

class McePlugin : public AbstractVoiceCallManagerPlugin {
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.mce")
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit McePlugin(QObject *parent = 0);
            ~McePlugin();

    QString pluginId() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void onVoiceCallsChanged();

private:
    class McePluginPrivate *d_ptr;
    Q_DECLARE_PRIVATE(McePlugin)
};

#endif // MCEPLUGIN_H
