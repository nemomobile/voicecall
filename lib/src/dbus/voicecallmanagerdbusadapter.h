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
    Q_CLASSINFO("D-Bus Interface", "org.nemomobile.voicecall.VoiceCallManager")

    Q_PROPERTY(QStringList providers READ providers NOTIFY providersChanged)
    Q_PROPERTY(QStringList voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)

    Q_PROPERTY(QString activeVoiceCall READ activeVoiceCall NOTIFY activeVoiceCallChanged)

    Q_PROPERTY(QString audioMode READ audioMode WRITE setAudioMode NOTIFY audioModeChanged)
    Q_PROPERTY(bool isAudioRouted READ isAudioRouted WRITE setAudioRouted NOTIFY audioRoutedChanged)
    Q_PROPERTY(bool isMicrophoneMuted READ isMicrophoneMuted WRITE setMuteMicrophone NOTIFY microphoneMutedChanged)
    Q_PROPERTY(bool isSpeakerMuted READ isSpeakerMuted WRITE setMuteSpeaker NOTIFY speakerMutedChanged)

public:
    explicit VoiceCallManagerDBusAdapter(QObject *parent = 0);
            ~VoiceCallManagerDBusAdapter();

    void configure(VoiceCallManagerInterface *manager);

    QStringList providers() const;
    QStringList voiceCalls() const;

    QString activeVoiceCall() const;

    QString audioMode() const;
    bool isAudioRouted() const;
    bool isMicrophoneMuted() const;
    bool isSpeakerMuted() const;

Q_SIGNALS:
    void error(const QString &message);
    void providersChanged();
    void voiceCallsChanged();

    void activeVoiceCallChanged();

    void audioModeChanged();
    void audioRoutedChanged();
    void microphoneMutedChanged();
    void speakerMutedChanged();

public Q_SLOTS:
    bool dial(const QString &provider, const QString &msisdn);

    void silenceRingtone();

    bool setAudioMode(const QString &mode);
    bool setAudioRouted(bool on = true);

    bool setMuteMicrophone(bool on = true);
    bool setMuteSpeaker(bool on = true);

    bool startDtmfTone(const QString &tone);
    bool stopDtmfTone();

private:
    class VoiceCallManagerDBusAdapterPrivate *d_ptr;

    Q_DECLARE_PRIVATE(VoiceCallManagerDBusAdapter)
};

#endif // VOICECALLMANAGERDBUSADAPTER_H
