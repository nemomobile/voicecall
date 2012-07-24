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
#include "declarativeview.h"

#include "voicecallmodel.h"
#include "voicecallprovidermodel.h"

#include "dbus/voicecallmanagerdbusproxy.h"

#include <QDeclarativeContext>
#include <QtSingleApplication>

class DeclarativeViewPrivate
{
public:
    DeclarativeViewPrivate()
        : manager(NULL), providers(NULL)
    {/*...*/}

    VoiceCallManagerDBusProxy   *manager;

    VoiceCallModel              *voicecalls;
    VoiceCallProviderModel      *providers;
};

DeclarativeView::DeclarativeView(QWidget *parent)
    : QDeclarativeView(parent), d(new DeclarativeViewPrivate)
{
    TRACE
    this->setAttribute(Qt::WA_OpaquePaintEvent);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    this->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    this->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    d->manager = new VoiceCallManagerDBusProxy(this);
    d->voicecalls = new VoiceCallModel(d->manager);
    d->providers = new VoiceCallProviderModel(d->manager);

    this->rootContext()->setContextProperty("VoiceCallManager", d->manager);
    this->rootContext()->setContextProperty("voicecalls", d->voicecalls);
    this->rootContext()->setContextProperty("providers", d->providers);

    QtSingleApplication *app = qobject_cast<QtSingleApplication*>(QApplication::instance());
    QObject::connect(d->manager, SIGNAL(activeVoiceCallChanged()), app, SLOT(activateWindow()));
    QObject::connect(d->manager, SIGNAL(activeVoiceCallChanged()), SLOT(showFullScreen()));
}

DeclarativeView::~DeclarativeView()
{
    TRACE
    delete this->d;
}
