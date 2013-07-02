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
#include "common.h"
#include "playbackmanagerplugin.h"

#include <voicecallmanagerinterface.h>

#include <QtDBus>
#include <QtPlugin>

#define ORG_MAEMO_PLAYBACK_DEST                     "org.maemo.Playback.Manager"
#define ORG_MAEMO_PLAYBACK_PATH                     "/org/maemo/Playback/Manager"
#define ORG_MAEMO_PLAYBACK_IFACE                    "org.maemo.Playback.Manager"
#define ORG_MAEMO_PLAYBACK_REQUEST_PRIVACY_OVERRIDE "RequestPrivacyOverride"
#define ORG_MAEMO_PLAYBACK_REQUEST_MUTE             "RequestMute"

class PlaybackManagerPluginPrivate
{
    Q_DECLARE_PUBLIC(PlaybackManagerPlugin)

public:
    PlaybackManagerPluginPrivate(PlaybackManagerPlugin *q)
        : q_ptr(q), manager(NULL)
    { /* ... */ }

    PlaybackManagerPlugin *q_ptr;

    VoiceCallManagerInterface *manager;
};

PlaybackManagerPlugin::PlaybackManagerPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d_ptr(new PlaybackManagerPluginPrivate(this))
{
    TRACE
}

PlaybackManagerPlugin::~PlaybackManagerPlugin()
{
    TRACE
    delete this->d_ptr;
}

QString PlaybackManagerPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

bool PlaybackManagerPlugin::initialize()
{
    TRACE
    return true;
}

bool PlaybackManagerPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(PlaybackManagerPlugin);
    d->manager = manager;

    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), SLOT(onVoiceCallsChanged()));

    QObject::connect(d->manager, SIGNAL(setAudioModeRequested(QString)), SLOT(setMode(QString)));
    QObject::connect(d->manager, SIGNAL(setMuteMicrophoneRequested(bool)), SLOT(setMuteMicrophone(bool)));
    QObject::connect(d->manager, SIGNAL(setMuteSpeakerRequested(bool)), SLOT(setMuteSpeaker(bool)));

    return true;
}

bool PlaybackManagerPlugin::start()
{
    TRACE
    return true;
}

bool PlaybackManagerPlugin::suspend()
{
    TRACE
    return true;
}

bool PlaybackManagerPlugin::resume()
{
    TRACE
    return true;
}

void PlaybackManagerPlugin::finalize()
{
    TRACE
}

void PlaybackManagerPlugin::setMode(const QString &mode)
{
    TRACE
    Q_D(PlaybackManagerPlugin);
    bool on = false;

    if(mode == "ihf")
    {
        on = true;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(ORG_MAEMO_PLAYBACK_DEST,
                                                          ORG_MAEMO_PLAYBACK_PATH,
                                                          ORG_MAEMO_PLAYBACK_IFACE,
                                                          ORG_MAEMO_PLAYBACK_REQUEST_PRIVACY_OVERRIDE);
    message << on;

    if(!QDBusConnection::sessionBus().send(message))
    {
        WARNING_T("Failed to send RequestPrivacyOverride method call.");
        return;
    }
    else
        DEBUG_T(on ? "Set PrivacyOverride true." : "Set PirvacyOverride false.");

    d->manager->onAudioModeChanged(mode);
}

void PlaybackManagerPlugin::setMuteMicrophone(bool on)
{
    TRACE
    Q_D(PlaybackManagerPlugin);
    QDBusMessage message = QDBusMessage::createMethodCall(ORG_MAEMO_PLAYBACK_DEST,
                                                          ORG_MAEMO_PLAYBACK_PATH,
                                                          ORG_MAEMO_PLAYBACK_IFACE,
                                                          ORG_MAEMO_PLAYBACK_REQUEST_MUTE);
    message << on;

    if(!QDBusConnection::sessionBus().send(message))
    {
        WARNING_T("Failed to send RequestMute method call.");
        return;
    }
    else
        DEBUG_T(on ? "Set Mute true." : "Set Mute false.");

    d->manager->onMuteMicrophoneChanged(on);
}

void PlaybackManagerPlugin::setMuteSpeaker(bool on)
{
    TRACE
    Q_D(PlaybackManagerPlugin);
    QDBusMessage message = QDBusMessage::createMethodCall(ORG_MAEMO_PLAYBACK_DEST,
                                                          ORG_MAEMO_PLAYBACK_PATH,
                                                          ORG_MAEMO_PLAYBACK_IFACE,
                                                          ORG_MAEMO_PLAYBACK_REQUEST_MUTE);
    message << !on;

    if(!QDBusConnection::sessionBus().send(message))
    {
        WARNING_T("Failed to send RequestMute method call.");
        return;
    }

    d->manager->onMuteSpeakerChanged(on);
}

void PlaybackManagerPlugin::onVoiceCallsChanged()
{
    TRACE
    Q_D(PlaybackManagerPlugin);

    if(d->manager->voiceCalls().empty())
    {
        d->manager->onAudioModeChanged("earpiece");
        d->manager->onMuteMicrophoneChanged(false);
        d->manager->onMuteSpeakerChanged(false);
    }
}

