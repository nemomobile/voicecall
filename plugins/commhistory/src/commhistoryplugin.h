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
#ifndef COMMHISTORYPLUGIN_H
#define COMMHISTORYPLUGIN_H

#include <CommHistory/EventModel>

#include <abstractvoicecallmanagerplugin.h>
#include <abstractvoicecallhandler.h>

class CommHistoryPlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.commhistory")
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit CommHistoryPlugin(QObject *parent = 0);
            ~CommHistoryPlugin();

    QString pluginId() const;

    CommHistory::EventModel* eventModel();

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void onVoiceCallAdded(AbstractVoiceCallHandler *handler);

private:
    class CommHistoryPluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(CommHistoryPlugin)
};

#endif // COMMHISTORYPLUGIN_H
