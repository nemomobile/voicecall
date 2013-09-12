/*
 * This file is a part of the Voice Call Manager Plugin project.
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
#ifndef ABSTRACTVOICECALLPROVIDER_H
#define ABSTRACTVOICECALLPROVIDER_H

#include <QObject>
#include "abstractvoicecallhandler.h"

class AbstractVoiceCallProvider : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString providerId READ providerId)
    Q_PROPERTY(QString providerType READ providerType)

    Q_PROPERTY(QList<AbstractVoiceCallHandler*> voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY error)

public:
    explicit AbstractVoiceCallProvider(QObject *parent = 0) : QObject(parent) {/* ... */}
    virtual ~AbstractVoiceCallProvider() {/* ... */}

    virtual QString providerId() const = 0;
    virtual QString providerType() const = 0;
    virtual QList<AbstractVoiceCallHandler*> voiceCalls() const = 0;
    virtual QString errorString() const = 0;

Q_SIGNALS:
    void error(QString);

    void voiceCallsChanged();
    void voiceCallAdded(AbstractVoiceCallHandler *handler);
    void voiceCallRemoved(const QString &handlerId);

public Q_SLOTS:
    virtual bool dial(const QString &msisdn) = 0;
};

#endif // ABSTRACTVOICECALLPROVIDER_H
