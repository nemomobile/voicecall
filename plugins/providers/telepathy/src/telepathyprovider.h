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
#ifndef TELEPATHYPROVIDER_H
#define TELEPATHYPROVIDER_H

#include <abstractvoicecallprovider.h>
#include <voicecallmanagerinterface.h>

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingOperation>

class TelepathyProvider : public AbstractVoiceCallProvider
{
    Q_OBJECT

    friend class TelepathyProviderPlugin;

public:
    explicit TelepathyProvider(Tp::AccountPtr account, VoiceCallManagerInterface *manager, QObject *parent = 0);
            ~TelepathyProvider();

    QString errorString() const;

    QString providerId() const;
    QString providerType() const;

    QList<AbstractVoiceCallHandler*> voiceCalls() const;

public Q_SLOTS:
    bool dial(const QString &msisdn);

protected Q_SLOTS:
    void onAccountBecomeReady(Tp::PendingOperation *op);
    void onAccountAvailabilityChanged();

    void onChannelRequestCreated(const Tp::ChannelRequestPtr &channelRequest);
    void onPendingRequestFinished(Tp::PendingOperation *op);
    void onDialFailed(const QString &errorName, const QString &errorMessage);
    void onHandlerInvalidated(const QString &errorName, const QString &errorMessage);

protected:
    void createHandler(Tp::ChannelPtr ch, const QDateTime &userActionTime);

private:
    class TelepathyProviderPrivate *d_ptr;

    Q_DISABLE_COPY(TelepathyProvider)
    Q_DECLARE_PRIVATE(TelepathyProvider)
};

#endif // TELEPATHYPROVIDER_H
