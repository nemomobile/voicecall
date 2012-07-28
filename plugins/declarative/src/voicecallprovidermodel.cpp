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
#include "voicecallprovidermodel.h"

#include "voicecallmanager.h"

class VoiceCallProviderData
{
public:
    VoiceCallProviderData() {/*..*/}
    VoiceCallProviderData(const QString &pId, const QString &pType, const QString &pLabel)
        : id(pId), type(pType), label(pLabel) {/*...*/}

    QString id;
    QString type;
    QString label;
};

class VoiceCallProviderModelPrivate
{
public:
    VoiceCallProviderModelPrivate(VoiceCallManager *pManager)
        : manager(pManager)
    {/*...*/}

    VoiceCallManager *manager;

    QHash<QString,VoiceCallProviderData> providers;
};

VoiceCallProviderModel::VoiceCallProviderModel(VoiceCallManager *manager)
    : QAbstractListModel(manager), d(new VoiceCallProviderModelPrivate(manager))
{
    TRACE

    QHash<int,QByteArray> roles;
    roles.insert(ROLE_ID, "id");
    roles.insert(ROLE_TYPE, "type");
    roles.insert(ROLE_LABEL, "name");
    this->setRoleNames(roles);

    QObject::connect(d->manager, SIGNAL(providersChanged()), SLOT(onProvidersChanged()));

    this->onProvidersChanged();
}

VoiceCallProviderModel::~VoiceCallProviderModel()
{
    TRACE
    delete this->d;
}

int VoiceCallProviderModel::count() const
{
    TRACE
    return this->rowCount(QModelIndex());
}

int VoiceCallProviderModel::rowCount(const QModelIndex &parent) const
{
    TRACE
    Q_UNUSED(parent)
    return d->providers.count();
}

QVariant VoiceCallProviderModel::data(const QModelIndex &index, int role) const
{
    TRACE
    if(!index.isValid() || index.row() >= d->providers.count()) return QVariant();

    QStringList keys = d->providers.keys();
    qSort(keys);

    VoiceCallProviderData provider = d->providers.value(keys.value(index.row()));

    switch(role)
    {
    case Qt::DisplayRole:
        return QVariant(provider.label);
    case ROLE_ID:
        return QVariant(provider.id);
    case ROLE_LABEL:
        return QVariant(provider.label);
    case ROLE_TYPE:
        return QVariant(provider.type);

    default:
        return QVariant();
    }
}

void VoiceCallProviderModel::onProvidersChanged()
{
    TRACE
    this->beginResetModel();

    d->providers.clear();
    foreach(QString provider, d->manager->interface()->property("providers").toStringList())
    {
        QStringList parts = provider.split(':');
        d->providers.insert(parts.first(), VoiceCallProviderData(parts.first(),
                                                                 parts.last(),
                                                                 parts.first()));
    }

    this->endResetModel();
    emit this->countChanged();
}

QString VoiceCallProviderModel::id(int index)
{
    TRACE
    if(index > d->providers.count()) return QString::null;

    QStringList keys = d->providers.keys();
    qSort(keys);

    VoiceCallProviderData provider = d->providers.value(keys.value(index));
    return provider.id;
}

QString VoiceCallProviderModel::type(int index)
{
    TRACE
    if(index > d->providers.count()) return QString::null;
    QStringList keys = d->providers.keys();
    qSort(keys);

    VoiceCallProviderData provider = d->providers.value(keys.value(index));
    return provider.type;
}

QString VoiceCallProviderModel::label(int index)
{
    TRACE
    if(index > d->providers.count()) return QString::null;
    QStringList keys = d->providers.keys();
    qSort(keys);

    VoiceCallProviderData provider = d->providers.value(keys.value(index));
    return provider.label;
}
