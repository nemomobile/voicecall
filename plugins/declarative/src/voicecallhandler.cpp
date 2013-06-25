#include "common.h"
#include "voicecallhandler.h"

#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingReply>

/*!
  \class VoiceCallHandler
  \brief This is the D-Bus proxy for communicating with the voice call manager
    from a declarative context, this interface specifically interfaces with
    the managers' voice call handler instances.
*/
class VoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(VoiceCallHandler)

public:
    VoiceCallHandlerPrivate(VoiceCallHandler *q, const QString &pHandlerId)
        : q_ptr(q), handlerId(pHandlerId), interface(NULL), connected(false)
    { /* ... */ }

    VoiceCallHandler *q_ptr;

    QString handlerId;

    QDBusInterface *interface;

    bool connected;
};

/*!
  Constructs a new proxy interface for the provided voice call handlerId.
*/
VoiceCallHandler::VoiceCallHandler(const QString &handlerId, QObject *parent)
    : QObject(parent), d_ptr(new VoiceCallHandlerPrivate(this, handlerId))
{
    TRACE
    Q_D(VoiceCallHandler);
    DEBUG_T(QString("Creating D-Bus interface to: ") + handlerId);
    d->interface = new QDBusInterface("org.nemomobile.voicecall",
                                      "/calls/" + handlerId,
                                      "org.nemomobile.voicecall.VoiceCall",
                                      QDBusConnection::sessionBus(),
                                      this);
    this->initialize();
}

VoiceCallHandler::~VoiceCallHandler()
{
    TRACE
    Q_D(VoiceCallHandler);
    delete d;
}

void VoiceCallHandler::initialize(bool notifyError)
{
    TRACE
    Q_D(VoiceCallHandler);
    bool success = false;

    if(d->interface->isValid())
    {
        success = true;
        success &= (bool)QObject::connect(d->interface, SIGNAL(error(QString)), SIGNAL(error(QString)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(statusChanged()), SIGNAL(statusChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(lineIdChanged()), SIGNAL(lineIdChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(durationChanged()), SIGNAL(durationChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(startedAtChanged()), SIGNAL(startedAtChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(emergencyChanged()), SIGNAL(emergencyChanged()));
        success &= (bool)QObject::connect(d->interface, SIGNAL(multipartyChanged()), SIGNAL(multipartyChanged()));
    }

    if(!(d->connected = success))
    {
        QTimer::singleShot(2000, this, SLOT(initialize()));
        if(notifyError) emit this->error("Failed to connect to VCM D-Bus service.");
    }
}

/*!
  Returns this voice calls' handler id.
 */
QString VoiceCallHandler::handlerId() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->handlerId;
}

/*!
  Returns this voice calls' provider id.
 */
QString VoiceCallHandler::providerId() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("providerId").toString();
}

/*!
  Returns this voice calls' call status.
 */
int VoiceCallHandler::status() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("status").toInt();
}

/*!
  Returns this voice calls' call status as a symbolic string.
 */
QString VoiceCallHandler::statusText() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("statusText").toString();
}

/*!
  Returns this voice calls' remote end-point line id.
 */
QString VoiceCallHandler::lineId() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("lineId").toString();
}

/*!
  Returns this voice calls' started at property.
 */
QDateTime VoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("startedAt").toDateTime();
}

/*!
  Returns this voice calls' duration property.
 */
int VoiceCallHandler::duration() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("duration").toInt();
}

/*!
  Returns this voice calls' incoming call flag property.
 */
bool VoiceCallHandler::isIncoming() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("isIncoming").toBool();
}

/*!
  Returns this voice calls' multiparty flag property.
 */
bool VoiceCallHandler::isMultiparty() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("isMultparty").toBool();
}

/*!
  Returns this voice calls' emergency flag property.
 */
bool VoiceCallHandler::isEmergency() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->interface->property("isEmergency").toBool();
}

/*!
  Initiates answering this call, if the call is an incoming call.
 */
void VoiceCallHandler::answer()
{
    TRACE
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("answer");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*!
  Initiates droping the call, unless the call is disconnected.
 */
void VoiceCallHandler::hangup()
{
    TRACE
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("hangup");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*!
  Initiates holding the call, unless the call is disconnected.
 */
void VoiceCallHandler::hold(bool on)
{
    TRACE
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("hold", on);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*!
  Initiates deflecting the call to the provided target phone number.
 */
void VoiceCallHandler::deflect(const QString &target)
{
    TRACE
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("deflect", target);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("sendDtmf", tones);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::onPendingCallFinished(QDBusPendingCallWatcher *watcher)
{
    TRACE
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        WARNING_T(QString::fromLatin1("Received error reply for member: %1 (%2)").arg(reply.reply().member()).arg(reply.error().message()));
        emit this->error(reply.error().message());
        watcher->deleteLater();
    } else {
        DEBUG_T(QString::fromLatin1("Received successful reply for member: %1").arg(reply.reply().member()));
    }
}
