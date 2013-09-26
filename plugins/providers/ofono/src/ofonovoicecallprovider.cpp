/*
 * This file is a part of the Voice Call Manager Ofono Plugin project.
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "common.h"
#include "ofonovoicecallhandler.h"
#include "ofonovoicecallprovider.h"

#include <qofonomodem.h>
#include <qofonovoicecallmanager.h>

class OfonoVoiceCallProviderPrivate
{
    Q_DECLARE_PUBLIC(OfonoVoiceCallProvider)

public:
    OfonoVoiceCallProviderPrivate(OfonoVoiceCallProvider *q, VoiceCallManagerInterface *pManager)
        : q_ptr(q), manager(pManager), ofonoManager(NULL), ofonoModem(NULL)
    { /* ... */ }

    OfonoVoiceCallProvider *q_ptr;

    VoiceCallManagerInterface *manager;

    QOfonoVoiceCallManager   *ofonoManager;
    QOfonoModem              *ofonoModem;
    QString modemPath;

    QHash<QString,OfonoVoiceCallHandler*> voiceCalls;

    QString errorString;
    void setError(const QString &errorString)
    {
        this->errorString = errorString;
        debugMessage(errorString);
    }

    void debugMessage(const QString &message)
    {
        DEBUG_T(QString("OfonoVoiceCallProvider(") + ofonoModem->modemPath() + "): " + message);
    }
};

OfonoVoiceCallProvider::OfonoVoiceCallProvider(const QString &path, VoiceCallManagerInterface *manager, QObject *parent)
    : AbstractVoiceCallProvider(parent), d_ptr(new OfonoVoiceCallProviderPrivate(this, manager))
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    d->modemPath = path;
    d->ofonoModem = new QOfonoModem(this);
    d->ofonoModem->setModemPath(path);
    connect(d->ofonoModem, SIGNAL(interfacesChanged(QStringList)), this, SLOT(interfacesChanged(QStringList)));

    if (d->ofonoModem->interfaces().contains(QLatin1String("org.ofono.VoiceCallManager")))
        initialize();
}

void OfonoVoiceCallProvider::initialize()
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    d->ofonoManager = new QOfonoVoiceCallManager(this);
    d->ofonoManager->setModemPath(d->modemPath);

    QObject::connect(d->ofonoManager, SIGNAL(callAdded(QString)), SLOT(onCallAdded(QString)));
    QObject::connect(d->ofonoManager, SIGNAL(callRemoved(QString)), SLOT(onCallRemoved(QString)));
    QObject::connect(d->ofonoManager, SIGNAL(dialComplete(bool)), SLOT(onDialComplete(bool)));
    QObject::connect(d->ofonoManager, SIGNAL(hangupComplete(bool)), SLOT(onHangupComplete(bool)));
    QObject::connect(d->ofonoManager, SIGNAL(transferComplete(bool)), SLOT(onTransferComplete(bool)));
    QObject::connect(d->ofonoManager, SIGNAL(swapCallsComplete(bool)), SLOT(onSwapCallsComplete(bool)));
    QObject::connect(d->ofonoManager, SIGNAL(releaseAndAnswerComplete(bool)), SLOT(onReleaseAndAnswerComplete(bool)));
    QObject::connect(d->ofonoManager, SIGNAL(holdAndAnswerComplete(bool)), SLOT(onHoldAndAnswerComplete(bool)));
    QObject::connect(d->ofonoManager, SIGNAL(privateChatComplete(bool, QStringList)), SLOT(onPrivateChatComplete(bool,QStringList)));
    QObject::connect(d->ofonoManager, SIGNAL(createMultipartyComplete(bool, QStringList)), SLOT(onCreateMultipartyComplete(bool,QStringList)));
    QObject::connect(d->ofonoManager, SIGNAL(hangupMultipartyComplete(bool)), SLOT(onHangupMultipartyComplete(bool)));

    foreach (const QString &call, d->ofonoManager->getCalls())
        onCallAdded(call);
}

OfonoVoiceCallProvider::~OfonoVoiceCallProvider()
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    delete d;
}

QString OfonoVoiceCallProvider::providerId() const
{
    TRACE
    Q_D(const OfonoVoiceCallProvider);
    return QString("ofono-") + d->modemPath;
}

QString OfonoVoiceCallProvider::providerType() const
{
    TRACE
    return "cellular";
}

QList<AbstractVoiceCallHandler*> OfonoVoiceCallProvider::voiceCalls() const
{
    TRACE
    Q_D(const OfonoVoiceCallProvider);
    QList<AbstractVoiceCallHandler*> results;

    foreach(AbstractVoiceCallHandler* handler, d->voiceCalls.values())
    {
        results.append(handler);
    }

    return results;
}

QString OfonoVoiceCallProvider::errorString() const
{
    TRACE
    Q_D(const OfonoVoiceCallProvider);
    return d->errorString;
}

bool OfonoVoiceCallProvider::dial(const QString &msisdn)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if(!d->ofonoManager || !d->ofonoManager->isValid())
    {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->dial(msisdn, "default");
    return true;
}

bool OfonoVoiceCallProvider::hangupAll() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->hangupAll();
    return true;
}

// XXX Not currently working with Nokia hardware?
bool OfonoVoiceCallProvider::sendTones(const QString &tones) {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->sendTones(tones);
    return true;
}

bool OfonoVoiceCallProvider::transfer() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->transfer();
    return true;
}

bool OfonoVoiceCallProvider::swapCalls() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->swapCalls();
    return true;
}

bool OfonoVoiceCallProvider::releaseAndAnswer() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->releaseAndAnswer();
    return true;
}

bool OfonoVoiceCallProvider::holdAndAnswer() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->holdAndAnswer();
    return true;
}

bool OfonoVoiceCallProvider::privateChat(const QString &path) {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->privateChat(path);
    return true;
}

bool OfonoVoiceCallProvider::createMultiparty() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->createMultiparty();
    return true;
}

bool OfonoVoiceCallProvider::hangupMultiparty() {
    TRACE
    Q_D(OfonoVoiceCallProvider);

    if(!d->ofonoManager || !d->ofonoManager->isValid()) {
        d->setError("ofono connection is not valid");
        return false;
    }

    d->ofonoManager->hangupMultiparty();
    return true;
}

QOfonoModem* OfonoVoiceCallProvider::modem() const
{
    TRACE
    Q_D(const OfonoVoiceCallProvider);
    return d->ofonoModem;
}

void OfonoVoiceCallProvider::interfacesChanged(const QStringList &interfaces)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    bool hasVoiceCallManager = interfaces.contains(QLatin1String("org.ofono.VoiceCallManager"));
    if (!hasVoiceCallManager && d->ofonoManager) {
        foreach (QString handler, d->voiceCalls.keys())
            onCallRemoved(handler);
        delete d->ofonoManager;
        d->ofonoManager = 0;
    } else if (hasVoiceCallManager && !d->ofonoManager) {
        initialize();
    }
}

void OfonoVoiceCallProvider::emergencyNumbersChanged(const QStringList &numbers)
{
    TRACE
    Q_UNUSED(numbers);
}

void OfonoVoiceCallProvider::onCallAdded(const QString &call)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if(d->voiceCalls.contains(call)) return;

    qDebug() << "Adding call handler " << call;
    OfonoVoiceCallHandler *handler = new OfonoVoiceCallHandler(d->manager->generateHandlerId(), call, this, d->ofonoManager);
    d->voiceCalls.insert(call, handler);

    emit this->voiceCallAdded(handler);
    emit this->voiceCallsChanged();
}

void OfonoVoiceCallProvider::onCallRemoved(const QString &call)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if(!d->voiceCalls.contains(call)) return;

    OfonoVoiceCallHandler *handler = d->voiceCalls.value(call);
    QString handlerId = handler->handlerId();
    d->voiceCalls.remove(call);
    handler->deleteLater();

    emit this->voiceCallRemoved(handlerId);
    emit this->voiceCallsChanged();
}

void OfonoVoiceCallProvider::onDialComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onHangupComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onTransferComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onSwapCallsComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onReleaseAndAnswerComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onHoldAndAnswerComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onPrivateChatComplete(bool status, const QStringList &calls)
{
    TRACE
    Q_UNUSED(calls);
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onCreateMultipartyComplete(bool status, const QStringList &calls)
{
    TRACE
    Q_UNUSED(calls);
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

void OfonoVoiceCallProvider::onHangupMultipartyComplete(bool status)
{
    TRACE
    Q_D(OfonoVoiceCallProvider);
    if (!d->ofonoManager) {
        d->setError("ofono connection is not valid");
        return;
    }
    if (!status)
        d->setError(d->ofonoManager->errorMessage());
}

// XXX NOT USED YET
void OfonoVoiceCallProvider::onBarringActive(const QString &type)
{
    TRACE
}

// XXX NOT USED YET
void OfonoVoiceCallProvider::onForwarded(const QString &type)
{
    TRACE
}
