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

    void onDialFinished(Tp::PendingOperation *op);
    void onHandlerInvalidated(const QString &errorName, const QString &errorMessage);

protected:
    void createHandler(Tp::ChannelPtr ch, const QDateTime &userActionTime);

private:
    class TelepathyProviderPrivate *d_ptr;

    Q_DISABLE_COPY(TelepathyProvider)
    Q_DECLARE_PRIVATE(TelepathyProvider)
};

#endif // TELEPATHYPROVIDER_H
