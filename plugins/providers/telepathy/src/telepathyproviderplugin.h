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
#ifndef TELEPATHYPROVIDERPLUGIN_H
#define TELEPATHYPROVIDERPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>

#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/MethodInvocationContext>

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingOperation>

class TelepathyProviderPlugin : public AbstractVoiceCallManagerPlugin, public Tp::AbstractClientHandler
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.telepathy")
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit TelepathyProviderPlugin(QObject *parent = 0);
            ~TelepathyProviderPlugin();

    QString pluginId() const;

    bool bypassApproval() const {return true;}
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                        const QDateTime &userActionTime,
                        const HandlerInfo &handlerInfo);

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void registerAccountProvider(Tp::AccountPtr account);
    void deregisterAccountProvider(Tp::AccountPtr account);

    void onAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccount(Tp::AccountPtr account);
    void onAccountInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);

private:
    class TelepathyProviderPluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(TelepathyProviderPlugin)
};

#endif // TELEPATHYPROVIDERPLUGIN_H
