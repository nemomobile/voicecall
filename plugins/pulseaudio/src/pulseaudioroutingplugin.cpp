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

#include <QSettings>
#include <QStringList>
#include <QtPlugin>

#define MODE_POLICY_FILEPATH  "/etc/voicecall/modes.ini"

#define MODE_KEY_SOURCE       "source"
#define MODE_KEY_SINK         "sink"
#define MODE_KEY_INPUT        "input"
#define MODE_KEY_INPUT_PORT   "input-port"
#define MODE_KEY_INPUT_SOURCE "input-source"
#define MODE_KEY_OUTPUT       "output"
#define MODE_KEY_OUTPUT_PORT  "output-port"
#define MODE_KEY_OUTPUT_SINK  "output-sink"

#define MODE_GLOBAL_NAME      ".global"
#define MODE_DEFAULT_NAME     ".default"

class PulseAudioRoutingPluginPrivate
{
    Q_DECLARE_PUBLIC(PulseAudioRoutingPlugin)

public:
    PulseAudioRoutingPluginPrivate(PulseAudioRoutingPlugin *q)
        : q_ptr(q), manager(NULL), control(NULL),
          isInitialized(false),
          isConfigured(false),
          isRouted(false),
          isMicrophoneMuted(false),
          isSpeakerMuted(false)
    {/*...*/}

    PulseAudioRoutingPlugin     *q_ptr;
    VoiceCallManagerInterface   *manager;

    PAControl                   *control;

    bool                         isInitialized;
    bool                         isConfigured;

    bool                         isRouted;
    bool                         isMicrophoneMuted;
    bool                         isSpeakerMuted;

    AudioRoutingMode                  mode;
    QHash<QString,AudioRoutingMode>   modes;
};

PulseAudioRoutingPlugin::PulseAudioRoutingPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d_ptr(new PulseAudioRoutingPluginPrivate(this))
{
    TRACE
}

PulseAudioRoutingPlugin::~PulseAudioRoutingPlugin()
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    delete d;
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

QString PulseAudioRoutingPlugin::mode() const
{
    TRACE
    Q_D(const PulseAudioRoutingPlugin);
    return d->mode.name;
}

bool PulseAudioRoutingPlugin::isModeValid(const AudioRoutingMode &mode)
{
    TRACE
    bool ok = true;

    if(mode.source.isNull() && mode.source.isEmpty())
    {
        WARNING_T("Mode has no source defined!");
        ok &= false;
    }
    if(mode.sink.isNull() && mode.sink.isEmpty())
    {
        WARNING_T("Mode has no sink defined!");
        ok &= false;
    }
    if(mode.input.isNull() && mode.input.isEmpty())
    {
        WARNING_T("Mode has no input defined!");
        ok &= false;
    }
    if(mode.input_port.isNull() && mode.input_port.isEmpty())
    {
        WARNING_T("Mode has no input port defined!");
        ok &= false;
    }
    if(mode.input_source.isNull() && mode.input_source.isEmpty())
    {
        WARNING_T("Mode has no input source defined!");
        ok &= false;
    }
    if(mode.output.isNull() && mode.output.isEmpty())
    {
        WARNING_T("Mode has no source defined!");
        ok &= false;
    }
    if(mode.output_port.isNull() && mode.output_port.isEmpty())
    {
        WARNING_T("Mode has no output port defined!");
        ok &= false;
    }
    if(mode.output_sink.isNull() && mode.output_sink.isEmpty())
    {
        WARNING_T("Mode has no output sink defined!");
        ok &= false;
    }

    return ok;
}

bool PulseAudioRoutingPlugin::initialize()
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    QSettings settings(MODE_POLICY_FILEPATH, QSettings::IniFormat, this);

    foreach(QString group, settings.childGroups())
    {
        AudioRoutingMode mode;
        settings.beginGroup(group);

        mode.name         = group;
        mode.source       = settings.value(MODE_KEY_SOURCE).toString();
        mode.sink         = settings.value(MODE_KEY_SINK).toString();
        mode.input        = settings.value(MODE_KEY_INPUT).toString();
        mode.input_port   = settings.value(MODE_KEY_INPUT_PORT).toString();
        mode.input_source = settings.value(MODE_KEY_INPUT_SOURCE).toString();
        mode.output       = settings.value(MODE_KEY_OUTPUT).toString();
        mode.output_port  = settings.value(MODE_KEY_OUTPUT_PORT).toString();
        mode.output_sink  = settings.value(MODE_KEY_OUTPUT_SINK).toString();

        DEBUG_T(QString("Read policy mode configuration: ") + mode.name);
        d->modes.insert(mode.name, mode);

        settings.endGroup();
    }

    // Check for at least a complete ".default" configuration.
    if(!d->modes.contains(MODE_DEFAULT_NAME))
    {
        WARNING_T("No default audio routing mode defined!");
        return false;
    }

    d->isInitialized = true;
    return true;
}

bool PulseAudioRoutingPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    if(!d->isInitialized)
    {
        WARNING_T("Not initialized!")
        return false;
    }

    d->manager = manager;
    d->control = PAControl::instance(d->manager);

    QObject::connect(d->manager, SIGNAL(setAudioModeRequested(QString)), SLOT(setMode(QString)));
    QObject::connect(d->manager, SIGNAL(setAudioRoutedRequested(bool)), SLOT(setRouted(bool)));
    QObject::connect(d->manager, SIGNAL(setMuteMicrophoneRequested(bool)), SLOT(setMuteMicrophone(bool)));
    QObject::connect(d->manager, SIGNAL(setMuteSpeakerRequested(bool)), SLOT(setMuteSpeaker(bool)));

    this->setMode(MODE_DEFAULT_NAME);

    d->isConfigured = true;
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

bool PulseAudioRoutingPlugin::setMode(const QString &mode)
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    if(!d->isInitialized)
    {
        WARNING_T("Not initialized!")
        return false;
    }

    AudioRoutingMode target;
    bool ok = true;

    if(!d->modes.contains(mode))
    {
        WARNING_T(QString("Failed to find audio routing mode: ") + mode);
        WARNING_T("Switching to default...");
        target = d->modes.value(MODE_DEFAULT_NAME);
        ok = false;
    }
    else
    {
        target = d->modes.value(mode);
    }

    if(target.name == d->mode.name)
    {
        DEBUG_T(QString("Audio routing mode already set to: ") + target.name);
        return true;
    }

    // Fill in global defaults for values missing from policy.
    if(d->modes.contains(MODE_GLOBAL_NAME))
    {
        AudioRoutingMode global = d->modes.value(MODE_GLOBAL_NAME);

        if(target.source.isNull() || target.source.isEmpty())
            target.source = global.source;

        if(target.sink.isNull() || target.sink.isEmpty())
            target.sink = global.sink;

        if(target.input.isNull() || target.input.isEmpty())
            target.input = global.input;

        if(target.input_port.isNull() || target.input_port.isEmpty())
            target.input_port = global.input_port;

        if(target.input_source.isNull() || target.input_source.isEmpty())
            target.input_source = global.input_source;

        if(target.output.isNull() || target.output.isEmpty())
            target.output = global.output;

        if(target.output_port.isNull() || target.output_port.isEmpty())
            target.output_port = global.output_port;

        if(target.output_sink.isNull() || target.output_sink.isEmpty())
            target.output_sink = global.output_sink;
    }

    ok &= this->isModeValid(target);

    if(ok)
    {
        DEBUG_T(QString("Successfully configured mode: ") + target.name);
        d->mode = target;
        emit this->modeChanged();

        if(d->isRouted)
        {
            d->isRouted = false; // To force re-routing of the audio.
            this->routeAudio();
        }
    }
    else
    {
        DEBUG_T(QString("Failed to configure mode: ") + target.name);
    }

    return ok;
}

bool PulseAudioRoutingPlugin::setMuteMicrophone(bool on)
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    if(!d->isInitialized)
    {
        WARNING_T("Not initialized!")
        return false;
    }

    if(!d->isRouted)
    {
        DEBUG_T("Audio not routed, returning.")
        return true;
    }

    d->control->toggleMuteSource(d->control->input_source, on);
    return true;
}

bool PulseAudioRoutingPlugin::setMuteSpeaker(bool on)
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    if(!d->isInitialized)
    {
        WARNING_T("Not initialized!")
        return false;
    }

    if(!d->isRouted)
    {
        DEBUG_T("Audio not routed, returning.")
        return true;
    }

    d->control->toggleMuteSink(d->control->output_sink, on);
    return true;
}

bool PulseAudioRoutingPlugin::setRouted(bool on)
{
    TRACE
    return on ? routeAudio() : unrouteAudio();
}

bool PulseAudioRoutingPlugin::routeAudio()
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    if(!d->isInitialized)
    {
        WARNING_T("Not initialized!")
        return false;
    }

    if(!this->isModeValid(d->mode))
    {
        WARNING_T("No valid mode configured!");
        return false;
    }

    if(d->isRouted)
    {
        DEBUG_T("Audio already routed.")
        return true;
    }

    int ok = true;

    if(!(d->control->source = d->control->findAlsaSource(d->mode.source)))
    {
        WARNING_T(QString("Failed to find source: ") + d->mode.source);
        ok &= false;
    }

    if(!(d->control->sink = d->control->findAlsaSink(d->mode.sink)))
    {
        WARNING_T(QString("Failed to find sink: ") + d->mode.sink);
        ok &= false;
    }
    if(!(d->control->input = d->control->findAlsaSource(d->mode.input)))
    {
        WARNING_T(QString("Failed to find input: ") + d->mode.input);
        ok &= false;
    }
    if(!(d->control->input_source = d->control->findAlsaSource(d->mode.input_source)))
    {
        WARNING_T(QString("Failed to find input source: ") + d->mode.input_source);
        ok &= false;
    }
    if(!(d->control->output = d->control->findAlsaSink(d->mode.output)))
    {
        WARNING_T(QString("Failed to find output: ") + d->mode.output);
        ok &= false;
    }
    if(!(d->control->output_sink = d->control->findAlsaSink(d->mode.output_sink)))
    {
        WARNING_T(QString("Failed to find output sink: ") + d->mode.output_sink);
        ok &= false;
    }

    d->control->input_port = d->mode.input_port;
    d->control->output_port = d->mode.output_port;

    if(ok)
    {
        DEBUG_T("Found all stream components, routing audio.");
        d->control->routeAudio();
    }
    else
    {
        DEBUG_T("Failed to find all stream components, failing.");
        if(d->isRouted) d->control->unrouteAudio();
        ok = false;
    }

    d->isRouted = ok;
    return ok;
}

bool PulseAudioRoutingPlugin::unrouteAudio()
{
    TRACE
    Q_D(PulseAudioRoutingPlugin);
    if(!d->isInitialized)
    {
        WARNING_T("Not initialized!")
        return false;
    }

    if(!d->isRouted)
    {
        DEBUG_T("Audio already not routed.")
        return true;
    }

    d->control->unrouteAudio();
    d->isRouted = false;

    return true;
}

Q_EXPORT_PLUGIN2(voicecall-pulseaudio-plugin, PulseAudioRoutingPlugin)
