#include "common.h"
#include "resourcepolicyroutingplugin.h"

#include "voicecallmanagerinterface.h"

#include <QtDBus>
#include <QVariant>
#include <QtPlugin>


struct AudioActionProp
{
    QString key;
    QDBusVariant value;

    AudioActionProp() {}
    AudioActionProp(const QString &k, const QDBusVariant &v)
        : key(k), value(v) { /* ... */ }

    bool isValid() const {return !key.isNull() && !key.isEmpty();}
};

typedef QList<AudioActionProp> AudioActionPropList;

Q_DECLARE_METATYPE(AudioActionProp)
Q_DECLARE_METATYPE(AudioActionPropList)

QDBusArgument &operator<<(QDBusArgument &arg, const AudioActionProp &prop)
{
    qDebug() << "action prop operator <<";

    arg.beginStructure();
    arg << prop.key << prop.value;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, AudioActionProp &prop)
{
    qDebug() << "action prop operator >>";
    return arg;
}

struct AudioActionObject
{
    AudioActionPropList props;

    bool isValid() const {return !props.isEmpty();}
};

typedef QList<AudioActionObject> AudioActionObjectList;

Q_DECLARE_METATYPE(AudioActionObject)
Q_DECLARE_METATYPE(AudioActionObjectList)

QDBusArgument &operator<<(QDBusArgument &arg, const AudioActionObject &obj)
{
    qDebug() << "action object operator <<";
    arg << obj.props;
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, AudioActionObject &obj)
{
    qDebug() << "action object operator >>";
    return arg;
}

struct AudioAction
{
    QString action;
    AudioActionObjectList objects;

    bool isValid() const {return !action.isNull() && !action.isEmpty();}
};

typedef QList<AudioAction> AudioActionList;

Q_DECLARE_METATYPE(AudioAction)
Q_DECLARE_METATYPE(AudioActionList)

QDBusArgument &operator<<(QDBusArgument &argument, const AudioAction &a)
{
    qDebug() << "action operator <<";

    argument.beginMap(QVariant::String, qMetaTypeId<AudioActionObjectList>());
    if(a.isValid())
    {
        argument.beginMapEntry();
        argument << a.action;
        argument << a.objects;
        argument.endMapEntry();
    }
    argument.endMap();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AudioAction &a)
{
    qDebug() << "action operator >>";
    return argument;
}

class ResourcePolicyRoutingPluginPrivate
{
    Q_DECLARE_PUBLIC(ResourcePolicyRoutingPlugin)

public:
    ResourcePolicyRoutingPluginPrivate(ResourcePolicyRoutingPlugin *q)
        : q_ptr(q), manager(NULL)
    { /* ... */ }

    ResourcePolicyRoutingPlugin *q_ptr;
    VoiceCallManagerInterface   *manager;

    QDBusInterface *iface;
};

ResourcePolicyRoutingPlugin::ResourcePolicyRoutingPlugin(QObject *parent) :
    AbstractVoiceCallManagerPlugin(parent), d_ptr(new ResourcePolicyRoutingPluginPrivate(this))
{
    TRACE
}

ResourcePolicyRoutingPlugin::~ResourcePolicyRoutingPlugin()
{
    TRACE
    delete this->d_ptr;
}

QString ResourcePolicyRoutingPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

QString ResourcePolicyRoutingPlugin::pluginVersion() const
{
    TRACE
    return PLUGIN_VERSION;
}

bool ResourcePolicyRoutingPlugin::initialize()
{
    TRACE
    qDBusRegisterMetaType<AudioActionProp>();
    qDBusRegisterMetaType<AudioActionPropList>();
    qDBusRegisterMetaType<AudioActionObject>();
    qDBusRegisterMetaType<AudioActionObjectList>();
    qDBusRegisterMetaType<AudioAction>();
    qDBusRegisterMetaType<AudioActionList>();
    return true;
}

bool ResourcePolicyRoutingPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(ResourcePolicyRoutingPlugin);
    d->manager = manager;

    QObject::connect(d->manager, SIGNAL(setAudioModeRequested(QString)), SLOT(setMode(QString)));
    QObject::connect(d->manager, SIGNAL(setMuteMicrophoneRequested(bool)), SLOT(setMuteMicrophone(bool)));
    QObject::connect(d->manager, SIGNAL(setMuteSpeakerRequested(bool)), SLOT(setMuteSpeaker(bool)));

    return true;
}

bool ResourcePolicyRoutingPlugin::start()
{
    TRACE
    return true;
}

bool ResourcePolicyRoutingPlugin::suspend()
{
    TRACE
    return true;
}

bool ResourcePolicyRoutingPlugin::resume()
{
    TRACE
    return true;
}

void ResourcePolicyRoutingPlugin::finalize()
{
    TRACE
}

void ResourcePolicyRoutingPlugin::setMode(const QString &mode)
{
    TRACE
    AudioAction act;
    AudioActionObject obj;
    AudioActionProp sink_device("device", QDBusVariant("earpiece"));
    AudioActionProp sink_mode("mode", QDBusVariant("na"));

    if(mode == "ihf")
    {
        sink_device.value = QDBusVariant("ihf");
        sink_mode.value = QDBusVariant("ihf");
    }

    act.action = "com.nokia.policy.audio_route";
    obj.props << AudioActionProp("type", QDBusVariant("sink"));
    obj.props << sink_device;
    obj.props << sink_mode;
    obj.props << AudioActionProp("hwid", QDBusVariant("na"));
    act.objects << obj;

    QDBusMessage message = QDBusMessage::createSignal("/com/nokia/policy/decision",
                                                      "com.nokia.policy",
                                                      "audio_actions");
    message << (uint)0;
    message << qVariantFromValue(act);

    if(!QDBusConnection::systemBus().send(message))
    {
        WARNING_T("Failed to send policy audio_actions signal.");
    }
}

void ResourcePolicyRoutingPlugin::setMuteMicrophone(bool on)
{
    TRACE
    AudioAction act;
    AudioActionObject a;

    act.action = "com.nokia.policy.audio_mute";

    a.props << AudioActionProp("device", QDBusVariant("microphone"));
    a.props << AudioActionProp("mute", on ? QDBusVariant("muted") : QDBusVariant("unmuted"));

    act.objects << a;

    QDBusMessage message = QDBusMessage::createSignal("/com/nokia/policy/decision",
                                                      "com.nokia.policy",
                                                      "audio_actions");
    message << (uint)0;
    message << qVariantFromValue(act);

    if(!QDBusConnection::systemBus().send(message))
    {
        WARNING_T("Failed to send policy audio_actions signal.");
    }
}

void ResourcePolicyRoutingPlugin::setMuteSpeaker(bool on)
{
    TRACE
            AudioAction act;
            AudioActionObject a;

            act.action = "com.nokia.policy.audio_mute";

            a.props << AudioActionProp("device", QDBusVariant("headset"));
            a.props << AudioActionProp("mute", on ? QDBusVariant("muted") : QDBusVariant("unmuted"));

            act.objects << a;

            QDBusMessage message = QDBusMessage::createSignal("/com/nokia/policy/decision",
                                                              "com.nokia.policy",
                                                              "audio_actions");
            message << (uint)0;
            message << qVariantFromValue(act);

            if(!QDBusConnection::systemBus().send(message))
            {
                WARNING_T("Failed to send policy audio_actions signal.");
            }
}

Q_EXPORT_PLUGIN2(resource-policy-routing-plugin, ResourcePolicyRoutingPlugin)
