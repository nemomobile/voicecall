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
#include "basicringtonenotificationprovider.h"

#include <voicecallmanagerinterface.h>

#include <QMediaPlayer>

class BasicRingtoneNotificationProviderPrivate
{
    Q_DECLARE_PUBLIC(BasicRingtoneNotificationProvider)

public:
    BasicRingtoneNotificationProviderPrivate(BasicRingtoneNotificationProvider *q)
        : q_ptr(q), player(NULL), currentCall(NULL), manager(NULL)
    {/* ... */}

    BasicRingtoneNotificationProvider *q_ptr;

    QMediaPlayer                *player;
    AbstractVoiceCallHandler    *currentCall;
    VoiceCallManagerInterface   *manager;
};

BasicRingtoneNotificationProvider::BasicRingtoneNotificationProvider(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent),
      d_ptr(new BasicRingtoneNotificationProviderPrivate(this))
{
    TRACE
}

BasicRingtoneNotificationProvider::~BasicRingtoneNotificationProvider()
{
    TRACE
    Q_D(BasicRingtoneNotificationProvider);
    delete d;
}

QString BasicRingtoneNotificationProvider::pluginId() const
{
    TRACE
    return "basic-ringtone-plugin";
}

QString BasicRingtoneNotificationProvider::pluginVersion() const
{
    TRACE
    return "0.0.0.1";
}

bool BasicRingtoneNotificationProvider::initialize()
{
    TRACE
    Q_D(BasicRingtoneNotificationProvider);
    if(d->player)
    {
        DEBUG_T("BasicRingtoneNotificationProvider: Already initialized!");
        return false;
    }

    d->player = new QMediaPlayer(this);
    return true;
}

bool BasicRingtoneNotificationProvider::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(BasicRingtoneNotificationProvider);

    d->manager = manager;
    QObject::connect(manager, SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)), SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
    QObject::connect(manager, SIGNAL(silenceRingtoneRequested()), d->player, SLOT(stop()));

    d->player->setMedia(QMediaContent(QUrl::fromLocalFile("/usr/share/voicecall/sounds/ring-1.wav")));
    d->player->setVolume(100);
    QObject::connect(d->player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), SLOT(onMediaPlayerMediaStatusChanged()));

    return true;
}

bool BasicRingtoneNotificationProvider::start()
{
    TRACE
    return true;
}

bool BasicRingtoneNotificationProvider::suspend()
{
    TRACE
    return true;
}

bool BasicRingtoneNotificationProvider::resume()
{
    TRACE
    return true;
}

void BasicRingtoneNotificationProvider::finalize()
{
    TRACE
}

void BasicRingtoneNotificationProvider::onVoiceCallAdded(AbstractVoiceCallHandler *handler)
{
    TRACE
    Q_D(BasicRingtoneNotificationProvider);

    QObject::connect(handler, SIGNAL(statusChanged()), SLOT(onVoiceCallStatusChanged()));
    d->currentCall = handler;
}

void BasicRingtoneNotificationProvider::onVoiceCallStatusChanged()
{
    TRACE
    Q_D(BasicRingtoneNotificationProvider);

    if(d->currentCall->status() != AbstractVoiceCallHandler::STATUS_INCOMING)
    {
        DEBUG_T("Disconnecting from handler.");
        QObject::disconnect(d->currentCall, SIGNAL(statusChanged()), this, SLOT(onVoiceCallStatusChanged()));

        d->player->stop();
        d->player->setPosition(0);

        d->currentCall = NULL;
    }
    else if(d->player->mediaStatus() != QMediaPlayer::PlayingState)
    {
        d->player->setPosition(0);
        d->player->play();
    }
}

void BasicRingtoneNotificationProvider::onMediaPlayerMediaStatusChanged()
{
    TRACE
    Q_D(BasicRingtoneNotificationProvider);

    if (d->player->mediaStatus() == QMediaPlayer::EndOfMedia)
    {
        d->player->setPosition(0);
        d->player->play();
    }
}
