#ifndef VOICECALLMANAGER_H
#define VOICECALLMANAGER_H

#include "voicecallhandler.h"

#include "voicecallmodel.h"
#include "voicecallprovidermodel.h"

#include <QObject>

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

class VoiceCallManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDBusInterface* interface READ interface)

    Q_PROPERTY(VoiceCallModel* voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)
    Q_PROPERTY(VoiceCallProviderModel* providers READ providers NOTIFY providersChanged)

    Q_PROPERTY(QString defaultProviderId READ defaultProviderId NOTIFY defaultProviderChanged)

    Q_PROPERTY(VoiceCallHandler* activeVoiceCall READ activeVoiceCall NOTIFY activeVoiceCallChanged)

    Q_PROPERTY(QString audioMode READ audioMode WRITE setAudioModeAsync NOTIFY audioModeChanged)
    Q_PROPERTY(bool isAudioRouted READ isAudioRouted WRITE setAudioRoutedAsync NOTIFY audioRoutedChanged)
    Q_PROPERTY(bool isMicrophoneMuted READ isMicrophoneMuted WRITE setMuteMicrophoneAsync NOTIFY microphoneMutedChanged)
    Q_PROPERTY(bool isSpeakerMuted READ isSpeakerMuted WRITE setMuteSpeakerAsync NOTIFY speakerMutedChanged)

public:
    explicit VoiceCallManager(QObject *parent = 0);
            ~VoiceCallManager();

    QDBusInterface* interface() const;

    VoiceCallModel* voiceCalls() const;
    VoiceCallProviderModel* providers() const;

    QString defaultProviderId() const;

    VoiceCallHandler* activeVoiceCall() const;

    void setAudioModeAsync(const QString &mode);
    void setAudioRoutedAsync(bool on);
    void setMuteMicrophoneAsync(bool on);
    void setMuteSpeakerAsync(bool on);


    QString audioMode() const;
    bool isAudioRouted() const;

    bool isMicrophoneMuted() const;
    bool isSpeakerMuted() const;

Q_SIGNALS:
    void error(const QString &message);

    void providersChanged();
    void voiceCallsChanged();

    void defaultProviderChanged();

    void activeVoiceCallChanged();

    void audioModeChanged();
    void audioRoutedChanged();
    void microphoneMutedChanged();
    void speakerMutedChanged();

public Q_SLOTS:
    void dial(const QString &providerId, const QString &msisdn);

    void silenceRingtone();

    bool setAudioMode(const QString &mode);
    bool setAudioRouted(bool on);
    bool setMuteMicrophone(bool on = true);
    bool setMuteSpeaker(bool on = true);

    bool startDtmfTone(const QString &tone);
    bool stopDtmfTone();

protected Q_SLOTS:
    void initialize(bool notifyError = false);

    void onProvidersChanged();
    void onVoiceCallsChanged();
    void onActiveVoiceCallChanged();

    void onPendingCallFinished(QDBusPendingCallWatcher *watcher);
    void onPendingSilenceFinished(QDBusPendingCallWatcher *watcher);

private:
    class VoiceCallManagerPrivate *d_ptr;

    Q_DISABLE_COPY(VoiceCallManager)
    Q_DECLARE_PRIVATE(VoiceCallManager)
};

#endif // VOICECALLMANAGER_H
