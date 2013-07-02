/*
 * This file is a part of the Voice Call Manager Ofono Plugin project.
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
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
#include "common.h"
#include "tonegeneratorplugin.h"

#include <QtPlugin>
#include <QDBusInterface>

class ToneGeneratorPluginPrivate
{
public:
    ToneGeneratorPluginPrivate()
        : manager(NULL), interface(NULL)
    {/*...*/}

    VoiceCallManagerInterface *manager;
    QDBusInterface *interface;
};

ToneGeneratorPlugin::ToneGeneratorPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d(new ToneGeneratorPluginPrivate)
{
    TRACE
}

ToneGeneratorPlugin::~ToneGeneratorPlugin()
{
    TRACE
    delete this->d;
}

QString ToneGeneratorPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

bool ToneGeneratorPlugin::initialize()
{
    TRACE
    d->interface = new QDBusInterface("com.Nokia.Telephony.Tones",
                                      "/com/Nokia/Telephony/Tones",
                                      "com.Nokia.Telephony.Tones",
                                      QDBusConnection::systemBus(),
                                      this);

    if(!d->interface->isValid())
    {
        qDebug() << d->interface->lastError().message();
        return false;
    }
    return true;
}

bool ToneGeneratorPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    d->manager = manager;

    QObject::connect(d->manager,
                     SIGNAL(startEventToneRequested(VoiceCallManagerInterface::ToneType,int)),
                     SLOT(onStartEventToneRequest(VoiceCallManagerInterface::ToneType,int)));
    QObject::connect(d->manager,
                     SIGNAL(startDtmfToneRequested(QString,int)),
                     SLOT(onStartDtmfToneRequest(QString,int)));
    QObject::connect(d->manager,
                     SIGNAL(stopEventToneRequested()),
                     SLOT(stopTone()));
    QObject::connect(d->manager,
                     SIGNAL(stopDtmfToneRequested()),
                     SLOT(stopTone()));

    return true;
}

bool ToneGeneratorPlugin::start()
{
    TRACE
    return true;
}

bool ToneGeneratorPlugin::suspend()
{
    TRACE
    return true;
}

bool ToneGeneratorPlugin::resume()
{
    TRACE
    return true;
}

void ToneGeneratorPlugin::finalize()
{
    TRACE
}

void ToneGeneratorPlugin::onStartEventToneRequest(VoiceCallManagerInterface::ToneType type, int volume)
{
    TRACE
    Q_UNUSED(type)
    Q_UNUSED(volume)
    if(!d->interface) return;
}

void ToneGeneratorPlugin::onStartDtmfToneRequest(const QString &tone, int volume)
{
    TRACE
    if(!d->interface) return;

    bool ok = true;
    unsigned int toneId = tone.toInt(&ok);

    if(!ok)
    {
        if (tone == "*") toneId = 10;
        else if(tone == "#") toneId = 11;
        else if(tone == "A") toneId = 12;
        else if(tone == "B") toneId = 13;
        else if(tone == "C") toneId = 14;
        else if(tone == "D") toneId = 15;
        else return;
    }

    qDebug() << "TONE ID:" << toneId;
    d->interface->call("StartEventTone", toneId, volume, (unsigned int)0);
}

void ToneGeneratorPlugin::stopTone()
{
    TRACE
    if(!d->interface) return;
    d->interface->call("StopTone");
}

