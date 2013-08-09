/*
 * This file is a part of the Voice Call Manager Pulse Audio Plugin project.
 * Copyright (c) 2012-2012 Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
#ifndef PULSEAUDIOROUTINGPLUGIN_H
#define PULSEAUDIOROUTINGPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>

struct AudioRoutingMode
{
    QString name;
    QString source; // source.voice - source and destination call audio.
    QString sink;   // sink.voice - source and destination call audio.
    QString input;
    QString input_port;
    QString input_source;
    QString output;
    QString output_port;
    QString output_sink;
};

class PulseAudioRoutingPlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.pulseaudio")
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    explicit PulseAudioRoutingPlugin(QObject *parent = 0);
            ~PulseAudioRoutingPlugin();
    
    QString pluginId() const;

    QString mode() const;

Q_SIGNALS:
    void modeChanged();

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

public Q_SLOTS:
    bool setMode(const QString &mode);
    bool setRouted(bool on = true);
    bool setMuteMicrophone(bool on = true);
    bool setMuteSpeaker(bool on = true);

protected:
    static bool isModeValid(const AudioRoutingMode &mode);

    bool routeAudio();
    bool unrouteAudio();

private:
    class PulseAudioRoutingPluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(PulseAudioRoutingPlugin)
};

#endif // PULSEAUDIOROUTINGPLUGIN_H
