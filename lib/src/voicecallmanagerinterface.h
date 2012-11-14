/*
 * This file is a part of the Voice Call Manager Plugin project.
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
#ifndef VOICECALLMANAGERINTERFACE_H
#define VOICECALLMANAGERINTERFACE_H

#include <QObject>
#include "abstractvoicecallprovider.h"

class VoiceCallManagerInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString errorString READ errorString WRITE setError NOTIFY error)

    Q_PROPERTY(QList<AbstractVoiceCallProvider*> providers READ providers NOTIFY providersChanged)

    Q_PROPERTY(int voiceCallCount READ voiceCallCount NOTIFY voiceCallsChanged)
    Q_PROPERTY(QList<AbstractVoiceCallHandler*> voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)

    Q_PROPERTY(AbstractVoiceCallHandler* activeVoiceCall READ activeVoiceCall NOTIFY activeVoiceCallChanged)

    Q_PROPERTY(QString audioMode READ audioMode WRITE setAudioMode NOTIFY audioModeChanged)
    Q_PROPERTY(bool isAudioRouted READ isAudioRouted WRITE setAudioRouted NOTIFY audioRoutedChanged)
    Q_PROPERTY(bool isMicrophoneMuted READ isMicrophoneMuted WRITE setMuteMicrophone NOTIFY microphoneMutedChanged)
    Q_PROPERTY(bool isSpeakerMuted READ isSpeakerMuted WRITE setMuteSpeaker NOTIFY speakerMutedChanged)

public:
    typedef enum {
        TONE_DIAL,
        TONE_BUSY,
        TONE_CONGEST,
        TONE_RADIO_ACK,
        TONE_RADIO_NA,
        TONE_ERROR,
        TONE_WAIT,
        TONE_RING
    } ToneType;

    explicit VoiceCallManagerInterface(QObject *parent = 0) : QObject(parent) {/*...*/}
    virtual ~VoiceCallManagerInterface() {/*...*/}

    virtual QList<AbstractVoiceCallProvider*> providers() const = 0;

    virtual QString generateHandlerId() = 0;

    virtual int voiceCallCount() const = 0;
    virtual QList<AbstractVoiceCallHandler*> voiceCalls() const = 0;

    virtual AbstractVoiceCallHandler* activeVoiceCall() const = 0;

    virtual QString audioMode() const = 0;
    virtual bool isAudioRouted() const = 0;
    virtual bool isMicrophoneMuted() const = 0;
    virtual bool isSpeakerMuted() const = 0;

    virtual QString errorString() const = 0;

Q_SIGNALS:
    void error(const QString &errorString);

    void providerAdded(AbstractVoiceCallProvider *provider);
    void providerRemoved(const QString &providerId);
    void providersChanged();

    void voiceCallAdded(AbstractVoiceCallHandler *handler);
    void voiceCallRemoved(const QString &handlerId);
    void voiceCallsChanged();

    void activeVoiceCallChanged();

    void audioModeChanged();
    void audioRoutedChanged();

    void microphoneMutedChanged();
    void speakerMutedChanged();

    void silenceRingtoneRequested();

    void setAudioModeRequested(const QString &mode);
    void setAudioRoutedRequested(bool on);

    void setMuteMicrophoneRequested(bool on);
    void setMuteSpeakerRequested(bool on);

    void startEventToneRequested(ToneType type, int volume);
    void stopEventToneRequested();

    void startDtmfToneRequested(const QString &tone, int volume);
    void stopDtmfToneRequested();

public Q_SLOTS:
    virtual void setError(const QString &errorString) = 0;

    virtual void appendProvider(AbstractVoiceCallProvider *provider) = 0;
    virtual void removeProvider(AbstractVoiceCallProvider *provider) = 0;

    virtual bool dial(const QString &providerId, const QString &msisdn) = 0;

    virtual void silenceRingtone() = 0;

    virtual void setAudioMode(const QString &mode) = 0;
    virtual void setAudioRouted(bool on = true) = 0;

    virtual void setMuteMicrophone(bool on = true) = 0;
    virtual void setMuteSpeaker(bool on = true) = 0;

    virtual void startEventTone(ToneType type, int volume) = 0;
    virtual void stopEventTone() = 0;

    virtual void startDtmfTone(const QString &tone, int volume) = 0;
    virtual void stopDtmfTone() = 0;
};

Q_DECLARE_INTERFACE(VoiceCallManagerInterface, "stage.rubyx.voicecall.Manager/1.0")

#endif // VOICECALLMANAGERINTERFACE_H
