/****************************************************************************
**
** Copyright (C) 2011-2012 Tom Swindell <t.swindell@rubyx.co.uk>
** All rights reserved.
**
** This file is part of the Voice Call UI project.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * The names of its contributors may NOT be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/
#include "common.h"
#include "voicecallmodel.h"

#include "voicecallmanager.h"

class VoiceCallModelPrivate
{
    Q_DECLARE_PUBLIC(VoiceCallModel)

public:
    VoiceCallModelPrivate(VoiceCallModel *q, VoiceCallManager *pManager)
        : q_ptr(q), manager(pManager)
    {/*...*/}

    VoiceCallModel *q_ptr;

    VoiceCallManager *manager;

    QList<VoiceCallHandler*> handlers;
};

VoiceCallModel::VoiceCallModel(VoiceCallManager *manager)
    : QAbstractListModel(manager), d_ptr(new VoiceCallModelPrivate(this, manager))
{
    TRACE
    Q_D(VoiceCallModel);
    QHash<int,QByteArray> roles;
    roles.insert(ROLE_ID, "id");
    roles.insert(ROLE_PROVIDER_ID, "providerId");
    roles.insert(ROLE_STATUS, "status");
    roles.insert(ROLE_LINE_ID, "lineId");
    roles.insert(ROLE_STARTED_AT, "startedAt");
    roles.insert(ROLE_IS_EMERGENCY, "isEmergency");
    roles.insert(ROLE_IS_MULTIPARTY, "isMultiparty");
    roles.insert(ROLE_INSTANCE, "instance");
    this->setRoleNames(roles);

    // Need to listen for signal on the manager, because it handles connectivity to VCM.
    QObject::connect(d->manager, SIGNAL(voiceCallsChanged()), SLOT(onVoiceCallsChanged()));
}

VoiceCallModel::~VoiceCallModel()
{
    TRACE
    Q_D(VoiceCallModel);
    delete d;
}

int VoiceCallModel::count() const
{
    TRACE
    return this->rowCount(QModelIndex());
}

int VoiceCallModel::rowCount(const QModelIndex &parent) const
{
    TRACE
    Q_D(const VoiceCallModel);
    Q_UNUSED(parent)
    return d->handlers.count();
}

QVariant VoiceCallModel::data(const QModelIndex &index, int role) const
{
    TRACE
    Q_D(const VoiceCallModel);
    if(!index.isValid() || index.row() >= d->handlers.count()) return QVariant();

    VoiceCallHandler *handler = this->instance(index.row());

    switch(role)
    {
    case Qt::DisplayRole:
        return QVariant(handler->lineId());
    case ROLE_PROVIDER_ID:
        return QVariant(handler->handlerId());
    case ROLE_STATUS:
        return QVariant(handler->status());
    case ROLE_LINE_ID:
        return QVariant(handler->lineId());
    case ROLE_STARTED_AT:
        return QVariant(handler->startedAt());
    case ROLE_IS_EMERGENCY:
        return QVariant(handler->isEmergency());
    case ROLE_IS_MULTIPARTY:
        return QVariant(handler->isMultiparty());
    case ROLE_INSTANCE:
        return QVariant::fromValue(static_cast<QObject*>(handler));
    default:
        return QVariant();
    }
}

void VoiceCallModel::onVoiceCallsChanged()
{
    TRACE
    Q_D(VoiceCallModel);
    QStringList nIds = d->manager->interface()->property("voiceCalls").toStringList();
    QStringList oIds;

    QStringList added;
    QStringList removed;

    // Map current call handlers to handler ids for easy indexing.
    foreach(VoiceCallHandler *handler, d->handlers)
    {
        oIds.append(handler->handlerId());
    }

    // Index new handlers to be added.
    foreach(QString nId, nIds)
    {
        if(!oIds.contains(nId)) added.append(nId);
    }

    // Index old handlers to be removed.
    foreach(QString oId, oIds)
    {
        if(!nIds.contains(oId)) removed.append(oId);
    }

    this->beginResetModel();

    // Remove handlers that need to be removed.
    foreach(QString removeId, removed)
    {
        VoiceCallHandler *handler = NULL;

        foreach(VoiceCallHandler *iHandler, d->handlers)
        {
            if(iHandler->handlerId() == removeId)
            {
                handler = iHandler;
                break;
            }
        }

        d->handlers.removeAll(handler);
        handler->deleteLater();
    }

    // Add handlers that need to be added.
    foreach(QString addId, added)
    {
        VoiceCallHandler *handler = new VoiceCallHandler(addId, this);
        d->handlers.append(handler);
    }

    this->endResetModel();

    emit this->countChanged();
}

VoiceCallHandler* VoiceCallModel::instance(int index) const
{
    TRACE
    Q_D(const VoiceCallModel);
    return d->handlers.value(index);
}

VoiceCallHandler* VoiceCallModel::instance(const QString &handlerId) const
{
    TRACE
    Q_D(const VoiceCallModel);
    foreach(VoiceCallHandler* handler, d->handlers)
    {
        if(handler->handlerId() == handlerId) return handler;
    }

    return NULL;
}
