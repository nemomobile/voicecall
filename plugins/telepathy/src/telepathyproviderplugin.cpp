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
#include "telepathyprovider.h"

#include <voicecallmanagerinterface.h>

#include <QtPlugin>

#include <TelepathyQt/Types>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountManager>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingStringList>

class TelepathyProviderPluginPrivate
{
    Q_DECLARE_PUBLIC(TelepathyProviderPlugin)

public:
    TelepathyProviderPluginPrivate(TelepathyProviderPlugin *q)
        : q_ptr(q), manager(NULL)
    {/* ... */}

    TelepathyProviderPlugin     *q_ptr;

    VoiceCallManagerInterface   *manager;

    Tp::AccountManagerPtr am;

    QHash<QString,TelepathyProvider*>    providers;
};

TelepathyProviderPlugin::TelepathyProviderPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent),
      d_ptr(new TelepathyProviderPluginPrivate(this))
{
    TRACE
}

TelepathyProviderPlugin::~TelepathyProviderPlugin()
{
    TRACE
    delete this->d_ptr;
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
    Q_D(TelepathyProviderPlugin);
    Tp::registerTypes();
    d->am = Tp::AccountManager::create();
    return true;
}

bool TelepathyProviderPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(TelepathyProviderPlugin);
    d->manager = manager;
    QObject::connect(d->am->becomeReady(),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onAccountManagerReady(Tp::PendingOperation*)));

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

void TelepathyProviderPlugin::onAccountManagerReady(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyProviderPlugin);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        return;
    }

    foreach(Tp::AccountPtr account, d->am->allAccounts())
    {
        qDebug() << "Found account:" << account->displayName();
        qDebug() << "\tManager Name:" << account->cmName();
        qDebug() << "\tProtocol Name:" << account->protocolName();
        qDebug() << "\tService Name:" << account->serviceName();

        if(account->protocolName() == "tel")
        {
            if(d->providers.contains(account->uniqueIdentifier()))
            {
                DEBUG_T("Ignoring already registered account.");
                continue;
            }

            DEBUG_T(QString("Registering provider: ") + account->uniqueIdentifier());
            TelepathyProvider *tp = new TelepathyProvider(account, d->manager, this);
            d->providers.insert(account->uniqueIdentifier(), tp);
            d->manager->appendProvider(tp);
        }
        else
        {
            DEBUG_T("Ignoring account due to unrecognised protocol.");
        }
    }
}

Q_EXPORT_PLUGIN2(voicecall-telepathy-plugin, TelepathyProviderPlugin)
