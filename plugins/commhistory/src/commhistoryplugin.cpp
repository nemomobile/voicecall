/*
* This file is a part of the Voice Call Manager Plugin project.
*
* Copyright (C) 2011-2015 Jolla Ltd
* Contact: Tom Swindell <tom.swindell@jollamobile.com>
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
#include "commhistoryplugin.h"

#include "calleventhandler.h"

#include <voicecallmanagerinterface.h>

class CommHistoryPluginPrivate
{
    Q_DECLARE_PUBLIC(CommHistoryPlugin)

public:
    CommHistoryPluginPrivate(CommHistoryPlugin *q)
        : q_ptr(q), manager(NULL), model(NULL)
    { /* ... */ }

    CommHistoryPlugin *q_ptr;
    VoiceCallManagerInterface *manager;

    CommHistory::EventModel *model;
};

CommHistoryPlugin::CommHistoryPlugin(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d_ptr(new CommHistoryPluginPrivate(this))
{
    TRACE
    Q_D(CommHistoryPlugin);
    d->model = new CommHistory::EventModel(this);
}

CommHistoryPlugin::~CommHistoryPlugin()
{
    TRACE
    delete this->d_ptr;
}

QString CommHistoryPlugin::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

bool CommHistoryPlugin::initialize()
{
    TRACE
    return true;
}

bool CommHistoryPlugin::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(CommHistoryPlugin);
    d->manager = manager;
    return true;
}

bool CommHistoryPlugin::start()
{
    TRACE
    Q_D(const CommHistoryPlugin);
    return QObject::connect(d->manager,
                            SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)),
                            this,
                            SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
}

bool CommHistoryPlugin::suspend()
{
    TRACE
    return true;
}

bool CommHistoryPlugin::resume()
{
    TRACE
    return true;
}

void CommHistoryPlugin::finalize()
{
    TRACE
}

CommHistory::EventModel* CommHistoryPlugin::eventModel()
{
    TRACE
    Q_D(CommHistoryPlugin);
    return d->model;
}

void CommHistoryPlugin::onVoiceCallAdded(AbstractVoiceCallHandler *handler)
{
    TRACE
    new CallEventHandler(handler, this);
}
