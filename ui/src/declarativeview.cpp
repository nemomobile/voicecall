/****************************************************************************
**
** Copyright (C) 2013 Jolla Ltd. <robin.burchell@jollamobile.com>
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
#include "dbusadaptor.h"

#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDBusConnection>
#include <QCoreApplication>

DeclarativeView::DeclarativeView(QWidget *parent)
    : QDeclarativeView(parent)
{
    TRACE

    new DBusAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.nemomobile.voicecall.ui");
    if (!QDBusConnection::sessionBus().registerObject("/", this))
        qWarning() << Q_FUNC_INFO << "Cannot register DBus object!";

    this->setAttribute(Qt::WA_OpaquePaintEvent);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    this->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    this->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    this->rootContext()->setContextProperty("__window", this);

    QObject::connect(this->engine(), SIGNAL(quit()), SLOT(close()));
}

DeclarativeView::~DeclarativeView()
{
    TRACE
}

void DeclarativeView::onActiveVoiceCallChanged()
{
    TRACE
    if(!this->isVisible())
    {
        this->rootContext()->setContextProperty("activationReason", "activeVoiceCallChanged");
        this->show();
    }
}

void DeclarativeView::show()
{
    TRACE

    this->activateWindow();

    if(QCoreApplication::arguments().contains("-no-fullscreen"))
    {
        this->setFixedSize(480, 854);
        QWidget::show();
    }
    else
    {
        this->showFullScreen();
    }
}
