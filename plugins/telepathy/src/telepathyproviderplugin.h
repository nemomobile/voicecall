#ifndef TELEPATHYPROVIDERPLUGIN_H
#define TELEPATHYPROVIDERPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>

#include <TelepathyQt/PendingOperation>

class TelepathyProviderPlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit TelepathyProviderPlugin(QObject *parent = 0);
            ~TelepathyProviderPlugin();

    QString pluginId() const;
    QString pluginVersion() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);

private:
    class TelepathyProviderPluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(TelepathyProviderPlugin)
};

#endif // TELEPATHYPROVIDERPLUGIN_H
