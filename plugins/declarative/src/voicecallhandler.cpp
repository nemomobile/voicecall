#include "common.h"
#include "voicecallhandler.h"

#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingReply>

class VoiceCallHandlerPrivate
{
public:
    VoiceCallHandlerPrivate(const QString &pHandlerId)
        : handlerId(pHandlerId), interface(NULL), connected(false)
    { /* ... */ }

    QString handlerId;

    QDBusInterface *interface;

    bool connected;
};

VoiceCallHandler::VoiceCallHandler(const QString &handlerId, QObject *parent)
    : QObject(parent), d(new VoiceCallHandlerPrivate(handlerId))
{
    TRACE
    DEBUG_T(QString("Creating D-Bus interface to: ") + handlerId);
    d->interface = new QDBusInterface("stage.rubyx.voicecall",
                                      "/calls/" + handlerId,
                                      "stage.rubyx.voicecall.VoiceCall",
                                      QDBusConnection::sessionBus(),
                                      this);
    this->initialize();
}

VoiceCallHandler::~VoiceCallHandler()
{
    TRACE
    delete this->d;
}

void VoiceCallHandler::initialize(bool notifyError)
{
    TRACE
    bool success = false;

    if(d->interface->isValid())
    {
        success = true;
        success &= QObject::connect(d->interface, SIGNAL(error(QString)), SIGNAL(error(QString)));
        success &= QObject::connect(d->interface, SIGNAL(statusChanged()), SIGNAL(statusChanged()));
        success &= QObject::connect(d->interface, SIGNAL(lineIdChanged()), SIGNAL(lineIdChanged()));
        success &= QObject::connect(d->interface, SIGNAL(durationChanged()), SIGNAL(durationChanged()));
        success &= QObject::connect(d->interface, SIGNAL(emergencyChanged()), SIGNAL(emergencyChanged()));
        success &= QObject::connect(d->interface, SIGNAL(multipartyChanged()), SIGNAL(multipartyChanged()));
    }

    if(!(d->connected = success))
    {
        QTimer::singleShot(2000, this, SLOT(initialize()));
        if(notifyError) emit this->error("Failed to connect to VCH D-Bus service.");
    }
}

QString VoiceCallHandler::handlerId() const
{
    TRACE
    return d->handlerId;
}

QString VoiceCallHandler::providerId() const
{
    TRACE
    return d->interface->property("providerId").toString();
}

int VoiceCallHandler::status() const
{
    TRACE
    return d->interface->property("status").toInt();
}

QString VoiceCallHandler::statusText() const
{
    TRACE
    return d->interface->property("statusText").toString();
}

QString VoiceCallHandler::lineId() const
{
    TRACE
    return d->interface->property("lineId").toString();
}

QDateTime VoiceCallHandler::startedAt() const
{
    TRACE
    return d->interface->property("startedAt").toDateTime();
}

int VoiceCallHandler::duration() const
{
    TRACE
    return d->interface->property("duration").toInt();
}

bool VoiceCallHandler::isMultiparty() const
{
    TRACE
    return d->interface->property("isMultparty").toBool();
}

bool VoiceCallHandler::isEmergency() const
{
    TRACE
    return d->interface->property("isEmergency").toBool();
}

void VoiceCallHandler::answer()
{
    TRACE
    QDBusPendingCall call = d->interface->asyncCall("answer");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::hangup()
{
    TRACE
    QDBusPendingCall call = d->interface->asyncCall("hangup");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::deflect(const QString &target)
{
    TRACE
    QDBusPendingCall call = d->interface->asyncCall("deflect", target);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::onPendingCallFinished(QDBusPendingCallWatcher *watcher)
{
    TRACE
    QDBusPendingReply<bool> reply = *watcher;

    if(reply.isError())
    {
        emit this->error(reply.error().message());
        watcher->deleteLater();
    }

    DEBUG_T(QString("Received successful reply for member: ") + reply.reply().member());
}
