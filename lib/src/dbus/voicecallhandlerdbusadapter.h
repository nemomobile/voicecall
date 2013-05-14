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
#ifndef VOICECALLHANDLERDBUSADAPTER_H
#define VOICECALLHANDLERDBUSADAPTER_H

#include "abstractvoicecallhandler.h"

#include <QDBusAbstractAdaptor>
#include <QDateTime>

class VoiceCallHandlerDBusAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "stage.rubyx.voicecall.VoiceCall")

    Q_PROPERTY(QString handlerId READ handlerId)
    Q_PROPERTY(QString providerId READ providerId)
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString lineId READ lineId NOTIFY lineIdChanged)
    Q_PROPERTY(QDateTime startedAt READ startedAt)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(bool isIncoming READ isIncoming)
    Q_PROPERTY(bool isEmergency READ isEmergency NOTIFY emergencyChanged)
    Q_PROPERTY(bool isMultiparty READ isMultiparty NOTIFY multipartyChanged)

public:
    explicit VoiceCallHandlerDBusAdapter(AbstractVoiceCallHandler *parent = 0);
            ~VoiceCallHandlerDBusAdapter();

    QString providerId() const;
    QString handlerId() const;
    int status() const;
    QString statusText() const;
    QString lineId() const;
    QDateTime startedAt() const;
    int duration() const;
    bool isIncoming() const;
    bool isMultiparty() const;
    bool isEmergency() const;

Q_SIGNALS:
    void error(const QString &message);
    void statusChanged();
    void lineIdChanged();
    void durationChanged();
    void emergencyChanged();
    void multipartyChanged();

public Q_SLOTS:
    bool answer();
    bool hangup();
    bool hold(bool on);
    bool deflect(const QString &target);
    void sendDtmf(const QString &tones);

protected:
    VoiceCallHandlerDBusAdapter(class VoiceCallHandlerDBusAdapterPrivate &d, AbstractVoiceCallHandler *parent = 0)
        : QDBusAbstractAdaptor(parent), d_ptr(&d)
    {/* ... */}

private:
    class VoiceCallHandlerDBusAdapterPrivate *d_ptr;

    Q_DECLARE_PRIVATE(VoiceCallHandlerDBusAdapter)
};

#endif // VOICECALLHANDLERDBUSADAPTER_H
