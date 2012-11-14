#ifndef AUDIOCALLPOLICYPROXY_H
#define AUDIOCALLPOLICYPROXY_H

#include <abstractvoicecallhandler.h>
#include <policy/resource-set.h>

class AudioCallPolicyProxy : public AbstractVoiceCallHandler
{
    Q_OBJECT

public:
    explicit AudioCallPolicyProxy(AbstractVoiceCallHandler *subject, QObject *parent = 0);
            ~AudioCallPolicyProxy();

    AbstractVoiceCallProvider* provider() const;

    QString handlerId() const;
    QString lineId() const;
    QDateTime startedAt() const;
    int duration() const;
    bool isMultiparty() const;
    bool isEmergency() const;

    VoiceCallStatus status() const;
    QString statusText() const;

public Q_SLOTS:
    void answer();
    void hangup();
    void deflect(const QString &target);
    void sendDtmf(const QString &tones);

protected Q_SLOTS:
    void invokeWithResources(QObject *receiver, const QString &method);

    void onResourceSetError(quint32 errno, const char* errorMessage);
    void onResourceSetGranted();
    void onResourceSetDenied();
    void onResourceSetLost();
    void onResourceSetBecameAvailable(QList<ResourcePolicy::ResourceType>);
    void onResourceSetReleased();
    void onResourceSetReleasedByManager();

protected:
    void createResourceSet();
    void deleteResourceSet();

    void connectResourceSet();
    void disconnectResourceSet();

private:
    class AudioCallPolicyProxyPrivate *d_ptr;

    Q_DECLARE_PRIVATE(AudioCallPolicyProxy)
};

#endif // AUDIOCALLPOLICYPROXY_H
