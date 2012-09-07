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

#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/ChannelClassSpec>

#include <TelepathyQt/AccountManager>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingStringList>

class TelepathyProviderPluginPrivate
{
    Q_DECLARE_PUBLIC(TelepathyProviderPlugin)

public:
    TelepathyProviderPluginPrivate(TelepathyProviderPlugin *q)
        : q_ptr(q), manager(NULL), tpClientHandler(NULL), tpClientRegistrar(NULL), am(NULL)
    {/* ... */}

    TelepathyProviderPlugin     *q_ptr;

    VoiceCallManagerInterface   *manager;

    Tp::AbstractClientPtr    tpClientHandler;
    Tp::ClientRegistrarPtr   tpClientRegistrar;

    Tp::AccountManagerPtr    am;

    QHash<QString,TelepathyProvider*>    providers;

    static const Tp::ChannelClassSpecList CHANNEL_SPECS;
};

//TODO: Properly ascertain what these different types actually related to (ring, sip, etc).
const Tp::ChannelClassSpecList TelepathyProviderPluginPrivate::CHANNEL_SPECS =
        (Tp::ChannelClassSpecList()
         << Tp::ChannelClassSpec::audioCall()
         << Tp::ChannelClassSpec::streamedMediaCall()
         << Tp::ChannelClassSpec::unnamedStreamedMediaCall()
         << Tp::ChannelClassSpec::streamedMediaAudioCall()
         << Tp::ChannelClassSpec::unnamedStreamedMediaCall()
         << Tp::ChannelClassSpec::incomingStreamTube()
         << Tp::ChannelClassSpec::incomingRoomStreamTube());

TelepathyProviderPlugin::TelepathyProviderPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent),
      Tp::AbstractClientHandler(TelepathyProviderPluginPrivate::CHANNEL_SPECS),
      d_ptr(new TelepathyProviderPluginPrivate(this))
{
    TRACE
}

TelepathyProviderPlugin::~TelepathyProviderPlugin()
{
    TRACE
    Q_D(TelepathyProviderPlugin);
    d->tpClientRegistrar->unregisterClient(d->tpClientHandler);
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

    d->tpClientHandler = Tp::AbstractClientPtr(this);
    d->tpClientRegistrar = Tp::ClientRegistrar::create();

    if(!d->tpClientRegistrar->registerClient(d->tpClientHandler, "voicecall", true))
    {
        WARNING_T("Failed to register telepathy 'voicecall' client!");
        return false;
    }

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

void TelepathyProviderPlugin::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                       const Tp::AccountPtr &account,
                                       const Tp::ConnectionPtr &connection,
                                       const QList<Tp::ChannelPtr> &channels,
                                       const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                       const QDateTime &userActionTime,
                                       const HandlerInfo &handlerInfo)
{
    TRACE
    Q_D(TelepathyProviderPlugin);
    Q_UNUSED(connection)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(handlerInfo)

    DEBUG_T(QString("Found %1 channel/s.").arg(channels.size()));

    if(!d->providers.contains(account.data()->uniqueIdentifier()))
    {
        DEBUG_T(QString("Ignoring channels as account '%1' is not registered.").arg(account.data()->uniqueIdentifier()));
        return;
    }

    TelepathyProvider *provider = d->providers.value(account.data()->uniqueIdentifier());
    DEBUG_T(QString("Found provider for account %1, invoking provider to create handlers.").arg(account.data()->uniqueIdentifier()));
    foreach(Tp::ChannelPtr ch, channels)
    {
        provider->createHandler(ch, userActionTime);
    }

    context->setFinished();
}

void TelepathyProviderPlugin::registerAccountProvider(Tp::AccountPtr account)
{
    TRACE
    Q_D(TelepathyProviderPlugin);

    if(d->providers.contains(account.data()->uniqueIdentifier()))
    {
        DEBUG_T(QString("Ignoring already registered account: %1").arg(account.data()->uniqueIdentifier()));
        return;
    }

    DEBUG_T(QString("Registering provider for account: %1").arg(account->uniqueIdentifier()));
    TelepathyProvider *tp = new TelepathyProvider(account, d->manager, this);
    d->providers.insert(account.data()->uniqueIdentifier(), tp);
}

void TelepathyProviderPlugin::deregisterAccountProvider(Tp::AccountPtr account)
{
    TRACE
    Q_D(TelepathyProviderPlugin);
    if(d->providers.contains(account.data()->uniqueIdentifier()))
    {
        DEBUG_T("Removing invalidated account from provider directory.");
        TelepathyProvider *provider = d->providers.value(account.data()->uniqueIdentifier());
        d->manager->removeProvider(provider);
        d->providers.remove(account.data()->uniqueIdentifier());
        provider->deleteLater();
    }
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
        this->onNewAccount(account);
    }

    QObject::connect(d->am.data(), SIGNAL(newAccount(Tp::AccountPtr)), SLOT(onNewAccount(Tp::AccountPtr)));
}

void TelepathyProviderPlugin::onNewAccount(Tp::AccountPtr account)
{
    TRACE
    qDebug() << "Found account:" << account->displayName();
    qDebug() << "\tManager Name:" << account->cmName();
    qDebug() << "\tProtocol Name:" << account->protocolName();
    qDebug() << "\tService Name:" << account->serviceName();

    if(account->protocolName() == "tel" || account->protocolName() == "sip")
    {
        this->registerAccountProvider(account);

        QObject::connect(account.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                         SLOT(onAccountInvalidated(Tp::DBusProxy*,QString,QString)));
    }
    else
    {
        DEBUG_T(QString("Ignoring account '%1' due to unrecognised protocol.").arg(account.data()->uniqueIdentifier()));
    }
}

void TelepathyProviderPlugin::onAccountInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
{
    TRACE
    Tp::AccountPtr account = Tp::AccountPtr(qobject_cast<Tp::Account*>(proxy));

    QObject::disconnect(account.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)), this, SLOT(onAccountInvalidated(Tp::DBusProxy*,QString,QString)));

    DEBUG_T(QString("Account: '%1' invalidated:%2: %3").arg(account.data()->uniqueIdentifier()).arg(errorName).arg(errorMessage));
    this->deregisterAccountProvider(account);
}

Q_EXPORT_PLUGIN2(voicecall-telepathy-plugin, TelepathyProviderPlugin)
