#ifndef TELEPATHYPROVIDERPLUGIN_H
#define TELEPATHYPROVIDERPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>

#include <TelepathyQt/AbstractClientHandler>

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingOperation>

class TelepathyProviderPlugin : public AbstractVoiceCallManagerPlugin, public Tp::AbstractClientHandler
{
    Q_OBJECT

    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit TelepathyProviderPlugin(QObject *parent = 0);
            ~TelepathyProviderPlugin();

    QString pluginId() const;
    QString pluginVersion() const;

    bool bypassApproval() const {return true;}
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                        const QDateTime &userActionTime,
                        const HandlerInfo &handlerInfo);

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void registerAccountProvider(Tp::AccountPtr account);
    void deregisterAccountProvider(Tp::AccountPtr account);

    void onAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccount(Tp::AccountPtr account);
    void onAccountInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);

private:
    class TelepathyProviderPluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(TelepathyProviderPlugin)
};

#endif // TELEPATHYPROVIDERPLUGIN_H
