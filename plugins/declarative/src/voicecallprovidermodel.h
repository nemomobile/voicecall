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
#ifndef VOICECALLPROVIDERMODEL_H
#define VOICECALLPROVIDERMODEL_H

#include <QAbstractTableModel>

class VoiceCallManager;

class VoiceCallProviderModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum {
        ROLE_ID = Qt::UserRole + 1,
        ROLE_TYPE,
        ROLE_LABEL
    };

    explicit VoiceCallProviderModel(VoiceCallManager *manager);
            ~VoiceCallProviderModel();

    int count() const;
    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

Q_SIGNALS:
    void countChanged();

public Q_SLOTS:
    QString id(int index) const;
    QString type(int index) const;
    QString label(int index) const;

protected:
    QHash<int, QByteArray> roleNames() const;

protected Q_SLOTS:
    void onProvidersChanged();

private:
    class VoiceCallProviderModelPrivate *d_ptr;

    Q_DECLARE_PRIVATE(VoiceCallProviderModel)
};

#endif // VOICECALLPROVIDERMODEL_H
