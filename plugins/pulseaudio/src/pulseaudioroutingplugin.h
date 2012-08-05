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

class PulseAudioRoutingPlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)
public:
    explicit PulseAudioRoutingPlugin(QObject *parent = 0);
            ~PulseAudioRoutingPlugin();
    
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
    void onSetMuteMicrophone(bool on = true);

private:
    class PulseAudioRoutingPluginPrivate *d;
};

#endif // PULSEAUDIOROUTINGPLUGIN_H
