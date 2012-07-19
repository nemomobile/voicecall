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
#include "basicvoicecallconfigurator.h"

#include "dbus/voicecallmanagerdbusservice.h"
#include "basicringtonenotificationprovider.h"

#include <QDir>
#include <QPluginLoader>

class BasicVoiceCallConfiguratorPrivate
{
public:
    BasicVoiceCallConfiguratorPrivate()
        : manager(NULL)
    {/* ... */}

    VoiceCallManagerInterface *manager;

    QHash<QString,AbstractVoiceCallManagerPlugin*> plugins;
};

BasicVoiceCallConfigurator::BasicVoiceCallConfigurator(QObject *parent)
    : QObject(parent), d(new BasicVoiceCallConfiguratorPrivate)
{
    TRACE
}

BasicVoiceCallConfigurator::~BasicVoiceCallConfigurator()
{
    TRACE
}

void BasicVoiceCallConfigurator::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    d->manager = manager;

    // Install statically linked plugins.
    this->installPlugin(new VoiceCallManagerDBusService(this));
    this->installPlugin(new BasicRingtoneNotificationProvider(this));

    QDir pluginPath("/usr/lib/voicecall/plugins");

    qDebug() << "Loading dynamic plugins from:" << pluginPath.absolutePath();
    foreach(QString plugin, pluginPath.entryList((QStringList() << "lib*plugin*so"),
                                                 QDir::NoDotAndDotDot | QDir::Files))
    {
        qDebug() << "Attempting to load dynamic plugin:" << pluginPath.absoluteFilePath(plugin);

        QPluginLoader loader(pluginPath.absoluteFilePath(plugin));
        QObject *instance = loader.instance();
        AbstractVoiceCallManagerPlugin *plugin = NULL;

        if(!instance)
        {
            qWarning() << "Failed to load plugin:" << loader.errorString();
            loader.unload();
            continue;
        }

        plugin = qobject_cast<AbstractVoiceCallManagerPlugin*>(instance);

        if(!plugin)
        {
            qWarning() << "Failed to load plugin: No manager plugin interface.";
            delete plugin;
            loader.unload();
            continue;
        }

        this->installPlugin(plugin);
    }
}

bool BasicVoiceCallConfigurator::installPlugin(AbstractVoiceCallManagerPlugin *plugin)
{
    TRACE
    qDebug() << "Attempting to install plugin:" << plugin->pluginId() + "/" + plugin->pluginVersion();

    if(d->plugins.contains(plugin->pluginId()))
    {
        qDebug() << "Plugin already installed";
        return false;
    }

    d->plugins.insert(plugin->pluginId(), plugin);

    plugin->initialize();
    plugin->configure(d->manager);
    plugin->start();
    return true;
}

void BasicVoiceCallConfigurator::removePlugin(AbstractVoiceCallManagerPlugin *plugin)
{
    TRACE
    if(!d->plugins.contains(plugin->pluginId())) return;

    plugin->suspend();
    plugin->finalize();

    d->plugins.remove(plugin->pluginId());
    plugin->deleteLater();
}
