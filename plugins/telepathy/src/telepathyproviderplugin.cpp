/*
 * This file is a part of the Voice Call Manager Telepathy Plugin project.
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
#include "telepathyproviderplugin.h"

#include <QtPlugin>

class TelepathyProviderPluginPrivate
{
    Q_DECLARE_PUBLIC(TelepathyProviderPlugin)

public:
    TelepathyProviderPluginPrivate(TelepathyProviderPlugin *q)
        : q_ptr(q), manager(NULL)
    {/* ... */}

    TelepathyProviderPlugin *q_ptr;

    VoiceCallManagerInterface   *manager;
};

TelepathyProviderPlugin::TelepathyProviderPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d_ptr(new TelepathyProviderPluginPrivate(this))
{
    TRACE
}

TelepathyProviderPlugin::~TelepathyProviderPlugin()
{
    TRACE
    Q_D(TelepathyProviderPlugin);
    delete d;
}

QString TelepathyProviderPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

QString TelepathyProviderPlugin::pluginVersion() const
{
    TRACE
    return PLUGIN_VERSION;
}

bool TelepathyProviderPlugin::initialize()
{
    TRACE
    return true;
}

bool TelepathyProviderPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(TelepathyProviderPlugin);

    d->manager = manager;

    return true;
}

bool TelepathyProviderPlugin::start()
{
    TRACE
    return true;
}

bool TelepathyProviderPlugin::suspend()
{
    TRACE
    return true;
}

bool TelepathyProviderPlugin::resume()
{
    TRACE
    return true;
}

void TelepathyProviderPlugin::finalize()
{
    TRACE
}

Q_EXPORT_PLUGIN2(voicecall-telepathy-plugin, TelepathyProviderPlugin)
