#include "common.h"
#include "voicecallhandler.h"

#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QVariantMap>

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
        , duration(0), status(0), emergency(false), multiparty(false)
        , forwarded(false), remoteHeld(false)
    { /* ... */ }

    VoiceCallHandler *q_ptr;

    QString handlerId;

    QDBusInterface *interface;

    bool connected;
    int duration;
    int status;
    QString statusText;
    QString lineId;
    QString providerId;
    QDateTime startedAt;
    bool emergency;
    bool multiparty;
    bool forwarded;
    bool remoteHeld;
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
        success &= (bool)QObject::connect(d->interface, SIGNAL(statusChanged(int,QString)), SLOT(onStatusChanged(int,QString)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(lineIdChanged(QString)), SLOT(onLineIdChanged(QString)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(durationChanged(int)), SLOT(onDurationChanged(int)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(startedAtChanged(QDateTime)), SLOT(onStartedAtChanged(QDateTime)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(emergencyChanged(bool)), SLOT(onEmergencyChanged(bool)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(multipartyChanged(bool)), SLOT(onMultipartyChanged(bool)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(forwardedChanged(bool)), SLOT(onForwardedChanged(bool)));
        success &= (bool)QObject::connect(d->interface, SIGNAL(remoteHeldChanged(bool)), SLOT(onRemoteHeldChanged(bool)));
    }

    if(!(d->connected = success))
    {
        QTimer::singleShot(2000, this, SLOT(initialize()));
        if(notifyError) emit this->error("Failed to connect to VCM D-Bus service.");
    } else {
        QDBusReply<QVariantMap> reply = d->interface->call("getProperties");
        if (reply.isValid()) {
            QVariantMap props = reply.value();
            d->providerId = props["providerId"].toString();
            d->duration = props["duration"].toInt();
            d->status = props["status"].toInt();
            d->statusText = props["statusText"].toString();
            d->lineId = props["lineId"].toString();
            d->startedAt = QDateTime::fromMSecsSinceEpoch(props["startedAt"].toULongLong());
            d->multiparty = props["isMultiparty"].toBool();
            d->emergency = props["isEmergency"].toBool();
            d->forwarded = props["isForwarded"].toBool();
            d->remoteHeld = props["isRemoteHeld"].toBool();
            emit durationChanged();
            emit statusChanged();
            emit lineIdChanged();
            emit startedAtChanged();
            emit multipartyChanged();
            emit emergencyChanged();
            emit forwardedChanged();
            emit isReadyChanged();
            emit isRemoteHeld();
        } else if (notifyError) {
            emit this->error("Failed to getProperties() from VCM D-Bus service.");
        }
    }
}

void VoiceCallHandler::onDurationChanged(int duration)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->duration = duration;
    emit durationChanged();
}

void VoiceCallHandler::onStatusChanged(int status, const QString &statusText)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->status = status;
    d->statusText = statusText;
    emit statusChanged();
}

void VoiceCallHandler::onLineIdChanged(const QString &lineId)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->lineId = lineId;
    emit lineIdChanged();
}

void VoiceCallHandler::onStartedAtChanged(const QDateTime &startedAt)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->startedAt = startedAt;
    emit startedAtChanged();
}

void VoiceCallHandler::onEmergencyChanged(bool emergency)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->emergency = emergency;
    emit emergencyChanged();
}

void VoiceCallHandler::onMultipartyChanged(bool multiparty)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->multiparty = multiparty;
    emit multipartyChanged();
}

void VoiceCallHandler::onForwardedChanged(bool forwarded)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->forwarded = forwarded;
    emit forwardedChanged();
}

void VoiceCallHandler::onRemoteHeldChanged(bool remoteHeld)
{
    TRACE
    Q_D(VoiceCallHandler);
    d->remoteHeld = remoteHeld;
    emit remoteHeldChanged();
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
    return d->providerId;
}

/*!
  Returns this voice calls' call status.
 */
int VoiceCallHandler::status() const
{
    Q_D(const VoiceCallHandler);
    return d->status;
}

/*!
  Returns this voice calls' call status as a symbolic string.
 */
QString VoiceCallHandler::statusText() const
{
    Q_D(const VoiceCallHandler);
    return d->statusText;
}

/*!
  Returns this voice calls' remote end-point line id.
 */
QString VoiceCallHandler::lineId() const
{
    Q_D(const VoiceCallHandler);
    return d->lineId;
}

/*!
  Returns this voice calls' started at property.
 */
QDateTime VoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->startedAt;
}

/*!
  Returns this voice calls' duration property.
 */
int VoiceCallHandler::duration() const
{
    Q_D(const VoiceCallHandler);
    return d->duration;
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
    return d->multiparty;
}

/*!
  Returns this voice calls' forwarded flag property.
 */
bool VoiceCallHandler::isForwarded() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->forwarded;
}

/*!
  Returns this voice calls' remote held flag property.
 */
bool VoiceCallHandler::isRemoteHeld() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->remoteHeld;
}

/*!
  Returns this voice calls' emergency flag property.
 */
bool VoiceCallHandler::isEmergency() const
{
    TRACE
    Q_D(const VoiceCallHandler);
    return d->emergency;
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
