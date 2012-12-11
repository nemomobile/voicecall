#ifndef RESOURCEPOLICYROUTINGPLUGIN_H
#define RESOURCEPOLICYROUTINGPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>

class ResourcePolicyRoutingPlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit ResourcePolicyRoutingPlugin(QObject *parent = 0);
            ~ResourcePolicyRoutingPlugin();

    QString pluginId() const;
    QString pluginVersion() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

public Q_SLOTS:
    void setMode(const QString &mode);
    void setMuteMicrophone(bool on = true);
    void setMuteSpeaker(bool on = true);

private:
    class ResourcePolicyRoutingPluginPrivate *d_ptr;

    Q_DECLARE_PRIVATE(ResourcePolicyRoutingPlugin)
};

#endif // RESOURCEPOLICYROUTINGPLUGIN_H
