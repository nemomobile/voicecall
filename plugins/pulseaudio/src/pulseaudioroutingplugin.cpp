/*
 * This file is a part of the Voice Call Manager Pulse Audio Plugin project.
 * Copyright (c) 2012-2012 Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
#include "common.h"
#include "pulseaudioroutingplugin.h"

#include "pacontrol.h"

#include <QtPlugin>

class PulseAudioRoutingPluginPrivate
{
public:
    PulseAudioRoutingPluginPrivate()
        : control(NULL), manager(NULL)
    {/*...*/}

    PAControl        *control;
    VoiceCallManagerInterface *manager;
};

PulseAudioRoutingPlugin::PulseAudioRoutingPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d(new PulseAudioRoutingPluginPrivate)
{
    TRACE
}

PulseAudioRoutingPlugin::~PulseAudioRoutingPlugin()
{
    TRACE
    delete this->d;
}

QString PulseAudioRoutingPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

QString PulseAudioRoutingPlugin::pluginVersion() const
{
    TRACE
    return PLUGIN_VERSION;
}

bool PulseAudioRoutingPlugin::initialize()
{
    TRACE
    return true;
}

bool PulseAudioRoutingPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    d->manager = manager;
    d->control = PAControl::instance(d->manager);
    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), d->control, SLOT(onCallsChanged()));
    return true;
}

bool PulseAudioRoutingPlugin::start()
{
    TRACE
    return true;
}

bool PulseAudioRoutingPlugin::suspend()
{
    TRACE
    return true;
}

bool PulseAudioRoutingPlugin::resume()
{
    TRACE
    return true;
}

void PulseAudioRoutingPlugin::finalize()
{
    TRACE
}

Q_EXPORT_PLUGIN2(voicecall-pulseaudio-plugin, PulseAudioRoutingPlugin)
