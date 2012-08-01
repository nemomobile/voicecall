#ifndef VOICECALLMANAGER_H
#define VOICECALLMANAGER_H

#include "voicecallhandler.h"

#include "voicecallmodel.h"
#include "voicecallprovidermodel.h"

#include <QDeclarativeItem>

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

class VoiceCallManager : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QDBusInterface* interface READ interface)

    Q_PROPERTY(VoiceCallModel* voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)
    Q_PROPERTY(VoiceCallProviderModel* providers READ providers NOTIFY providersChanged)

    Q_PROPERTY(QString defaultProviderId READ defaultProviderId NOTIFY defaultProviderChanged)

    Q_PROPERTY(VoiceCallHandler* activeVoiceCall READ activeVoiceCall NOTIFY activeVoiceCallChanged)

    Q_PROPERTY(bool muteRingtone READ muteRingtone WRITE setMuteRingtone NOTIFY muteRingtoneChanged)

public:
    explicit VoiceCallManager(QDeclarativeItem *parent = 0);
            ~VoiceCallManager();

    QDBusInterface* interface() const;

    VoiceCallModel* voiceCalls() const;
    VoiceCallProviderModel* providers() const;

    QString defaultProviderId() const;

    VoiceCallHandler* activeVoiceCall() const;

    bool muteRingtone() const;

Q_SIGNALS:
    void error(const QString &message);

    void providersChanged();
    void voiceCallsChanged();

    void defaultProviderChanged();

    void activeVoiceCallChanged();

    void muteRingtoneChanged();

public Q_SLOTS:
    void dial(const QString &providerId, const QString &msisdn);

    bool setMuteRingtone(bool on = true);

    bool startDtmfTone(const QString &tone);
    bool stopDtmfTone();

protected Q_SLOTS:
    void initialize(bool notifyError = false);

    void onVoiceCallsChanged();
    void onProvidersChanged();
    void onActiveVoiceCallChanged();

    void onPendingCallFinished(QDBusPendingCallWatcher *watcher);

private:
    class VoiceCallManagerPrivate *d;

    Q_DISABLE_COPY(VoiceCallManager)
};

QML_DECLARE_TYPE(VoiceCallManager)

#endif // VOICECALLMANAGER_H
