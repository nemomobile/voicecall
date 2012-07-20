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
#ifndef VOICECALLMANAGERDBUSADAPTER_H
#define VOICECALLMANAGERDBUSADAPTER_H

#include <QStringList>
#include <QDBusAbstractAdaptor>

#include "abstractvoicecallhandler.h"
#include "voicecallmanagerinterface.h"

class VoiceCallManager;

class VoiceCallManagerDBusAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "stage.rubyx.voicecall.VoiceCallManager")

    Q_PROPERTY(QStringList providers READ providers NOTIFY providersChanged)
    Q_PROPERTY(QString activeVoiceCall READ activeVoiceCall NOTIFY activeVoiceCallChanged)
    Q_PROPERTY(QStringList voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)

public:
    explicit VoiceCallManagerDBusAdapter(QObject *parent = 0);
            ~VoiceCallManagerDBusAdapter();

    void configure(VoiceCallManagerInterface *manager);

    QStringList providers() const;
    QString activeVoiceCall() const;
    QStringList voiceCalls() const;

Q_SIGNALS:
    void error(const QString &message);
    void providersChanged();

    void activeVoiceCallChanged();
    void voiceCallsChanged();
    void incomingVoiceCall(const QString &providerId, const QString &callId);

public Q_SLOTS:
    void dial(const QString &provider, const QString &msisdn);

    void silenceNotifications();

    void startDtmfTone(const QString &tone, int volume);
    void stopDtmfTone();

protected Q_SLOTS:
    void onIncomingVoiceCall(AbstractVoiceCallHandler *handler);

private:
    class VoiceCallManagerDBusAdapterPrivate *d;
};

#endif // VOICECALLMANAGERDBUSADAPTER_H
