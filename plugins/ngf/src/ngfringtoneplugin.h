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
#ifndef NGFRINGTONEPLUGIN_H
#define NGFRINGTONEPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>
#include <abstractvoicecallhandler.h>

class NgfRingtonePlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.ngf")
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit NgfRingtonePlugin(QObject *parent = 0);
            ~NgfRingtonePlugin();

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
    void onVoiceCallStatusChanged();
    void onSilenceRingtoneRequested();

protected Q_SLOTS:
    void onConnectionStatus(bool connected);
    void onEventFailed(quint32 eventId);
    void onEventCompleted(quint32 eventId);
    void onEventPlaying(quint32 eventId);
    void onEventPaused(quint32 eventId);

private:
    class NgfRingtonePluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(NgfRingtonePlugin)
};

#endif // NGFRINGTONEPLUGIN_H
