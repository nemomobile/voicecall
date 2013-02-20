/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#ifndef AUDIOCALLPOLICYPROXY_H
#define AUDIOCALLPOLICYPROXY_H

#include <abstractvoicecallhandler.h>
#include <policy/resource-set.h>

class AudioCallPolicyProxy : public AbstractVoiceCallHandler
{
    Q_OBJECT

public:
    explicit AudioCallPolicyProxy(AbstractVoiceCallHandler *subject, QObject *parent = 0);
            ~AudioCallPolicyProxy();

    AbstractVoiceCallProvider* provider() const;

    QString handlerId() const;
    QString lineId() const;
    QDateTime startedAt() const;
    int duration() const;
    bool isIncoming() const;
    bool isMultiparty() const;
    bool isEmergency() const;

    VoiceCallStatus status() const;
    QString statusText() const;

public Q_SLOTS:
    void answer();
    void hangup();
    void hold(bool on = true);
    void deflect(const QString &target);
    void sendDtmf(const QString &tones);

protected Q_SLOTS:
    void invokeWithResources(QObject *receiver, const QString &method);

    void onResourceSetError(quint32 errno, const char* errorMessage);
    void onResourceSetGranted();
    void onResourceSetDenied();
    void onResourceSetLost();
    void onResourceSetBecameAvailable(QList<ResourcePolicy::ResourceType>);
    void onResourceSetReleased();
    void onResourceSetReleasedByManager();

protected:
    void createResourceSet();
    void deleteResourceSet();

    void connectResourceSet();
    void disconnectResourceSet();

private:
    class AudioCallPolicyProxyPrivate *d_ptr;

    Q_DECLARE_PRIVATE(AudioCallPolicyProxy)
};

#endif // AUDIOCALLPOLICYPROXY_H
