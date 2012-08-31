#ifndef TELEPATHYPROVIDER_H
#define TELEPATHYPROVIDER_H

#include <abstractvoicecallprovider.h>
#include <voicecallmanagerinterface.h>

#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/PendingOperation>

class TelepathyProvider : public AbstractVoiceCallProvider, public Tp::AbstractClientHandler
{
    Q_OBJECT
public:
    explicit TelepathyProvider(Tp::AccountPtr account, VoiceCallManagerInterface *manager, QObject *parent = 0);
            ~TelepathyProvider();

    QString errorString() const;

    QString providerId() const;
    QString providerType() const;

    QList<AbstractVoiceCallHandler*> voiceCalls() const;

    bool bypassApproval() const {return true;}
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                        const QDateTime &userActionTime,
                        const HandlerInfo &handlerInfo);

public Q_SLOTS:
    bool dial(const QString &msisdn);

protected Q_SLOTS:
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
