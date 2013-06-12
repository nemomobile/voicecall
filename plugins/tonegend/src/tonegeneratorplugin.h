#ifndef TONEGENERATORPLUGIN_H
#define TONEGENERATORPLUGIN_H

#include <abstractvoicecallmanagerplugin.h>
#include <voicecallmanagerinterface.h>

class ToneGeneratorPlugin : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.tonegend")
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)

public:
    explicit ToneGeneratorPlugin(QObject *parent = 0);
            ~ToneGeneratorPlugin();

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
    void onStartEventToneRequest(VoiceCallManagerInterface::ToneType type, int volume);
    void onStartDtmfToneRequest(const QString &tone, int volume);
    void stopTone();

private:
    class ToneGeneratorPluginPrivate *d;
};

#endif // TONEGENERATORPLUGIN_H
