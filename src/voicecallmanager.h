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
#ifndef VOICECALLMANAGER_H
#define VOICECALLMANAGER_H

#include <voicecallmanagerinterface.h>

class VoiceCallManager : public VoiceCallManagerInterface
{
    Q_OBJECT

public:
    explicit VoiceCallManager(QObject *parent = 0);
            ~VoiceCallManager();

    QList<AbstractVoiceCallProvider*> providers() const;

    AbstractVoiceCallHandler* activeVoiceCall() const;

    int voiceCallCount() const;
    QList<AbstractVoiceCallHandler*> voiceCalls() const;

    QString errorString() const;

Q_SIGNALS:
    void incomingVoiceCall(AbstractVoiceCallHandler *handler);

public Q_SLOTS:
    void setError(const QString &errorString);

    void appendProvider(AbstractVoiceCallProvider *provider);
    void removeProvider(AbstractVoiceCallProvider *provider);

    bool dial(const QString &providerId, const QString &msisdn);

    void silenceNotifications();

protected Q_SLOTS:
    void onVoiceCallAdded(AbstractVoiceCallHandler *handler);
    void onVoiceCallRemoved(const QString &handlerId);

private:
    class VoiceCallManagerPrivate *d;
};

#endif // VOICECALLMANAGER_H
