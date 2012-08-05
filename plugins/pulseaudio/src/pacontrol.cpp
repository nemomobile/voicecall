/*
 * hfdialer - Hands Free Voice Call Manager
 * Copyright (c) 2012, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "common.h"
#include "pacontrol.h"
#include <string.h>
#include <QStringBuilder>
#include <QList>
#include <QDebug>
#include <QTimer>

// Define our pulse audio loop and connection variables
static PAControl* paControl = NULL;
// Create a mainloop API and connection to the default server
static pa_glib_mainloop *pa_ml = NULL;

static void pa_subscribed_events_cb(pa_context *c, enum pa_subscription_event_type t, uint32_t , void *);

static void operation_callback(pa_context *c, int success, void *userdata)
{
    Q_UNUSED(c);
    Q_UNUSED(userdata);

    if (!success)
    {
        DEBUG_T(QString("Operation Failed"));
        paControl->setErrorMsg(QString("Operation Failed"));
    }
}

static void module_callback(pa_context *c, uint32_t index, void *userdata)
{
    Q_UNUSED(c);
    Q_UNUSED(userdata);
    if (index == PA_INVALID_INDEX)
    {
        DEBUG_T(QString("Load module failed"));
        paControl->setErrorMsg(QString("Load module failed"));
    }
}

static void pa_sourcelist_cb(pa_context *c,
                             const pa_source_info *l,
                             int is_last,
                             void *userdata)
{
    Q_UNUSED(c);
    Q_UNUSED(userdata);
    PADevice *source;

    if (is_last > 0)
    {
        //end of the list
        paControl->release();
        return;
    }

    DEBUG_T(QString("pa_sourcelist_cb() Source added: ") + l->name);
    source = new PADevice();
    source->index = l->index;
    if(l->name != NULL)
        source->name = l->name;
    if(l->description != NULL)
        source->description = l->description;
    paControl->addSource(source);
}

static void pa_sinklist_cb(pa_context *c,
                           const pa_sink_info *l,
                           int is_last,
                           void *userdata)
{
    Q_UNUSED(c);
    Q_UNUSED(userdata);
    PADevice *sink;

    if (is_last > 0)
    {
        //end of the list
        paControl->release();
        return;
    }

    sink = new PADevice();
    sink->index = l->index;
    if(l->name != NULL)
        sink->name = l->name;
    if(l->description != NULL)
        sink->description = l->description;
    paControl->addSink(sink);
}

static void pa_modulelist_cb(pa_context *c,
                             const pa_module_info *i,
                             int is_last,
                             void *userdata)
{
    Q_UNUSED(c);
    Q_UNUSED(userdata);
    PAModule *module;

    if (is_last > 0)
    {
        //end of the list
        paControl->release();
        return;
    }

    module = new PAModule();
    module->index = i->index;
    if(i->name != NULL)
        module->name = i->name;
    if(i->argument != NULL)
        module->argument = i->argument;
    paControl->addModule(module);
}

static void pa_state_cb(pa_context *c, void *)
{
    pa_context_state_t state = pa_context_get_state(c);
    if(state == PA_CONTEXT_READY)
    {
        paControl->setState(true);
        pa_context_set_subscribe_callback(c, pa_subscribed_events_cb, NULL);
        pa_operation *o;
        if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                       (PA_SUBSCRIPTION_MASK_MODULE|
                                        PA_SUBSCRIPTION_MASK_SINK|
                                        PA_SUBSCRIPTION_MASK_SOURCE|
                                        PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                        PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT), NULL, NULL))) {
            WARNING_T("pa_context_subscribe() failed");
        }
        if(o) pa_operation_unref(o);
        ///Get an initial list of sinks, sources and modules.
        paControl->addRef();
        pa_context_get_sink_info_list(c, pa_sinklist_cb, NULL);
        paControl->addRef();
        pa_context_get_source_info_list(c, pa_sourcelist_cb, NULL);
        paControl->addRef();
        pa_context_get_module_info_list(c, pa_modulelist_cb, NULL);
    }
    else if(state == PA_CONTEXT_FAILED)
    {
        ///Pulseaudio crashed?
        paControl->setState(false);
    }
    else
    {
        DEBUG_T(QString("PA state: ") + (int)state);
    }
}

static void pa_subscribed_events_cb(pa_context *c, enum pa_subscription_event_type t, uint32_t , void *)
{
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
    case PA_SUBSCRIPTION_EVENT_SINK:
        DEBUG_T("PA_SUBSCRIPTION_EVENT_SINK event triggered");
        foreach(PADevice* dev, paControl->sinkList)
        {
            delete dev;
        }
        paControl->sinkList.clear();
        paControl->addRef();
        pa_context_get_sink_info_list(c, pa_sinklist_cb, NULL);
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE:
        DEBUG_T("PA_SUBSCRIPTION_EVENT_SOURCE event triggered");
        foreach(PADevice* dev, paControl->sourceList)
        {
            delete dev;
        }
        paControl->sourceList.clear();
        paControl->addRef();
        pa_context_get_source_info_list(c, pa_sourcelist_cb, NULL);
        break;
    case PA_SUBSCRIPTION_EVENT_MODULE:
        DEBUG_T("PA_SUBSCRIPTION_EVENT_MODULE event triggered");
        foreach(PAModule* dev, paControl->moduleList)
        {
            delete dev;
        }
        paControl->moduleList.clear();
        paControl->addRef();
        pa_context_get_module_info_list(c, pa_modulelist_cb, NULL);
        break;
    }
}

PAControl::PAControl(VoiceCallManagerInterface *manager, QObject *parent)
    : QObject(parent)
{
    m_manager = manager;
    currentSource = NULL;
    currentSink = NULL;
    status = SUCCESS;
    m_paState = false;
    m_isInitialized = false;

}

PAControl::~PAControl()
{
    paCleanup();

    foreach (PADevice *source, sourceList)
    {
        DEBUG_T("delete source");
        delete source;
    }
    foreach (PADevice *sink, sinkList)
    {
        DEBUG_T("delete sink");
        delete sink;
    }
    foreach (PAModule *module, moduleList)
    {
        DEBUG_T("delete module");
        delete module;
    }

    DEBUG_T("~PAControl()");
    paControl = NULL;
}

PAControl* PAControl::instance(VoiceCallManagerInterface *manager)
{
    if(!paControl) paControl = new PAControl(manager);
    return paControl;
}

void PAControl::paInit()
{
    DEBUG_T("paInit()");

    // Create a mainloop API and connection to the default server
    if(!pa_ml) pa_ml = pa_glib_mainloop_new(NULL);

    pa_ctx = pa_context_new(pa_glib_mainloop_get_api(pa_ml),
                            "stage.rubyx.voicecall");

    // This function connects to the pulse server
    if (pa_context_connect(pa_ctx,
                           NULL,
                           PA_CONTEXT_NOFAIL, NULL) < 0)
    {
        qCritical("PulseAudioService: pa_context_connect() failed");
        paCleanup();
        return;
    }

    m_refCounter = 0;
    m_connected = false;
    m_audioRouted = false;
    m_btSourceReady =false;
    m_btSinkReady = false;
    m_isInitialized = true;

    pa_context_set_state_callback(pa_ctx, pa_state_cb, NULL);
}

void PAControl::paCleanup()
{
    DEBUG_T("paCleanup()");
    if(pa_ctx)
        pa_context_disconnect(pa_ctx);
    if(pa_ctx)
        pa_context_unref(pa_ctx);
}

void PAControl::setState(bool state)
{
    m_paState = state;
    if(state == false)
    {
        emit paFailed();
    }
}

void PAControl::addRef()
{
    m_refCounter++;
}

void PAControl::release()
{
    m_refCounter--;
    Q_ASSERT(m_refCounter >= 0);
}

void PAControl::reconnect()
{
    DEBUG_T("Pulseaudio: reconnect()");
    if(paControl)
        delete paControl;
    paControl = new PAControl(m_manager);
}



PADevice* PAControl::findBluezSource()
{
    if (sourceList.size() == 0)
    {
        addRef();
        pa_op = pa_context_get_source_info_list(pa_ctx, pa_sourcelist_cb, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        return NULL;
    }

    foreach (PADevice *source, sourceList)
    {
        QString name = source->name;

        if (name.startsWith(QString("bluez_source."), Qt::CaseSensitive))
        {
            DEBUG_T(QString("   Matched Bluez source: ") + name);
            return source;
        }
    }

    DEBUG_T("Bluez source: none found");
    return NULL;
}

PADevice*  PAControl::findBluezSink()
{
    if (sinkList.size() == 0)
    {
        addRef();
        pa_op = pa_context_get_sink_info_list(pa_ctx, pa_sinklist_cb, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        return NULL;
    }

    foreach (PADevice *sink, sinkList)
    {
        QString name = sink->name;

        if (name.startsWith(QString("bluez_sink."), Qt::CaseSensitive))
        {
            DEBUG_T(QString("   Matched Bluez sink: ") + name);
            return sink;
        }
    }

    DEBUG_T("Bluez sink: none found");
    return NULL;
}

PADevice* PAControl::findAlsaSource(QString alsasource)
{
    if (sourceList.size() == 0)
    {
        addRef();
        pa_op = pa_context_get_source_info_list(pa_ctx, pa_sourcelist_cb, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        return NULL;
    }

    foreach (PADevice *source, sourceList)
    {
        DEBUG_T(QString("Alsa source: ") + source->name);
        QString name = source->name;

        if (!alsasource.isNull() && !alsasource.isEmpty())
        {
            // if alsa source name is provided
            if (alsasource == name)
            {
                DEBUG_T(QString("   Matched Alsa source: ") + name);
                return source;
            }
        } else if (name.startsWith(QString("alsa_input."), Qt::CaseSensitive) &&
                   name.endsWith(QString("analog-stereo"), Qt::CaseSensitive) &&
                   !name.contains(QString("timb")))
        {
            // this is default behavior, it will try to look up one
            DEBUG_T(QString("   Matched Alsa source: ") + name);
            return source;
        }
    }

    DEBUG_T("Alsa source: none found");
    return NULL;
}

PADevice*  PAControl::findAlsaSink(QString alsasink)
{
    if (sinkList.size() == 0)
    {
        addRef();
        pa_op = pa_context_get_sink_info_list(pa_ctx, pa_sinklist_cb, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        return NULL;
    }

    foreach (PADevice *sink, sinkList)
    {
        DEBUG_T(QString("Alsa sink: ") + sink->name);
        QString name = sink->name;

        if (!alsasink.isNull() && !alsasink.isEmpty())
        {
            // if alsa sink name is provided
            if (alsasink == name)
            {
                DEBUG_T(QString("   Matched Alsa sink: ") + name);
                return sink;
            }
        } else if (name.startsWith(QString("alsa_output."), Qt::CaseSensitive) &&
                   name.endsWith(QString("analog-stereo"), Qt::CaseSensitive) &&
                   !name.contains(QString("timb")))
        {
            // this is default behavior, it will try to look up one
            DEBUG_T(QString("   Matched Alsa sink: ") + name);
            return sink;
        }
    }

    DEBUG_T("Alsa sink: none found");
    return NULL;
}

PAModule* PAControl::findModule(QString name, QString pattern)
{
    if (moduleList.size() == 0)
    {
        addRef();
        pa_op = pa_context_get_module_info_list(pa_ctx, pa_modulelist_cb, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        return NULL;
    }

    foreach (PAModule *module, moduleList)
    {
        if (module->name.contains(name) && module->argument.contains(pattern))
        {
            DEBUG_T(QString("   Matched module: ") + module->name);
            DEBUG_T(QString("   argument: ") + module->argument);
            return module;
        }
    }

    DEBUG_T("Module: none found");
    return NULL;
}

QList<PAModule*> PAControl::getAllModules()
{
    if (moduleList.size() == 0)
    {
        addRef();
        pa_op = pa_context_get_module_info_list(pa_ctx, pa_modulelist_cb, NULL);
        if(pa_op) pa_operation_unref(pa_op);
    }

    return moduleList;
}

void PAControl::addSource(PADevice* device)
{
    foreach(PADevice* dev, sourceList)
    {
        if(dev->name == device->name)
        {
            delete device;
            return; /// already exists
        }
    }

    sourceList.append(device);
    emit sourceAppeared(device);
}

void PAControl::addSink(PADevice* device)
{
    foreach(PADevice* dev, sinkList)
    {
        if(dev->name == device->name)
        {
            delete device;
            return; /// already exists
        }
    }

    sinkList.append(device);
    emit sinkAppeared(device);
}

void PAControl::addModule(PAModule *module)
{
    foreach(PAModule* dev, moduleList)
    {
        if(dev->name == module->name && dev->index == module->index)
        {
            delete module;
            return; /// already exists
        }
    }

    moduleList.append(module);
    emit moduleAppeared(module);
}

void PAControl::routeSourceWithSink(PADevice *source, PADevice *sink)
{
    DEBUG_T(QString("Routing from ") + source->name + " to " + sink->name);

    if (source != NULL && sink != NULL)
    {
        QString arg = "source=\"" % source->name % "\" sink=\"" % sink->name % "\"";

        pa_op = pa_context_load_module(pa_ctx, "module-loopback", arg.toAscii().data(), module_callback, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        DEBUG_T(QString("load-module module-loopback ") + arg);
    }
}

void PAControl::toggleMuteSource(PADevice *source, bool isMute)
{
    if (source != NULL)
    {
        pa_op =pa_context_set_source_mute_by_name(pa_ctx, source->name.toAscii().data(), isMute, operation_callback, NULL);
        if(pa_op) pa_operation_unref(pa_op);

        DEBUG_T(QString("set source mute ") + source->name + QString(" to ") + isMute);
    }
}

void  PAControl::unloadModule(PAModule* module)
{
    if (module != NULL && module->index >= 0)
    {
        pa_op = pa_context_unload_module(pa_ctx, module->index, operation_callback, NULL);
        if(pa_op) pa_operation_unref(pa_op);
        DEBUG_T(QString("unload-module module-loopback ") +  QString(module->name) + QString(" at index ") + module->index);
    }
}

PAStatus PAControl::getStatus()
{
    return this->status;
}

void PAControl::setErrorMsg(QString msg)
{
    if (msg != NULL)
    {
        this->status = ERROR;
        this->errorMsg = msg;
    }
}

QString PAControl::getErrorMsg()
{
    return this->errorMsg;
}

void PAControl::routeAudio()
{
    PADevice* source;
    PADevice* sink;
    PADevice* mic;
    PADevice* speaker;

    if (m_audioRouted)
    {
        DEBUG_T(QString("Audio already routed"));
        return;
    }

    if (m_refCounter > 0)
    {
        DEBUG_T("PA callback not finished, retry");
        QTimer::singleShot(1000, this, SLOT(routeAudio()));
        return;
    }

    DEBUG_T("Route audio");
    source = paControl->findBluezSource();
    sink = paControl->findBluezSink();

    if(source == NULL || sink == NULL)
    {
        DEBUG_T("Bluez source or speaker not found");
        return;
    }

    QString alsaSourceName = "";//MGConfItem(QString("/apps/dialer/alsasource")).value().toString();
    QString alsaSinkName = "";//MGConfItem(QString("/apps/dialer/alsasink")).value().toString();

    mic = paControl->findAlsaSource(alsaSourceName);
    speaker = paControl->findAlsaSink(alsaSinkName);

    if (mic != NULL and speaker != NULL)
    {
        paControl->routeSourceWithSink(source, speaker);
        paControl->routeSourceWithSink(mic, sink);
        currentSource = mic;
        currentSink = speaker;
        DEBUG_T("Create loopback modules successful");
    }
    else
    {
        DEBUG_T("Alsa source and speaker not found");
    }

    m_audioRouted = true;
    disconnect(this, SIGNAL(sourceAppeared(PADevice*)));
    disconnect(this, SIGNAL(sinkAppeared(PADevice*)));
}

void PAControl::unrouteAudio()
{
    DEBUG_T("Unroute audio");
    PAControl* paControl = PAControl::instance(m_manager);

    QList<PAModule*> mlist = paControl->getAllModules();
    foreach(PAModule *module, mlist)
    {
        if (module->name.contains("module-loopback") &&
            module->argument.contains("bluez") &&
            module->argument.contains("alsa")) {
            DEBUG_T(QString("Found loopback module, index: ") + module->index);
            paControl->unloadModule(module);
            DEBUG_T("Remove loopback module successful");
        }
    }

    currentSource = NULL;
    currentSink = NULL;
    m_audioRouted = false;
    m_btSourceReady = false;
    m_btSinkReady = false;
}

void PAControl::onSourceAppeared(PADevice* device)
{
    if(m_manager->voiceCallCount() == 0)
    {
        DEBUG_T("no calls active, ignore");
        return;
    }

    if(device->name.contains("bluez_source"))
    {
        m_btSourceReady = true;
    }

    if(!m_audioRouted && m_btSourceReady && m_btSinkReady)
    {
        DEBUG_T("Route microphone and speakers");
        routeAudio();
    }
}

void PAControl::onSinkAppeared(PADevice* device)
{
    if(m_manager->voiceCallCount() == 0)
    {
        DEBUG_T("no calls active, ignore");
        return;
    }

    if((device)->name.contains("bluez_sink"))
    {
        m_btSinkReady = true;
    }

    if(!m_audioRouted && m_btSourceReady && m_btSinkReady)
    {
        DEBUG_T("Route microphone and speakers");
        routeAudio();
    }
}

void PAControl::onCallsChanged()
{
    TRACE;
    if(!m_isInitialized) this->paInit();

    if (m_manager->voiceCallCount() > 1)
    {
        // new call is dialing or phone is picked up
        DEBUG_T("PAControl: new call in progress");

        if(m_audioRouted)
        {
            DEBUG_T("Audio already routed");
            return;
        }

        if(m_btSourceReady && m_btSinkReady)
        {
            DEBUG_T("Route microphone and speakers");
            routeAudio();
        }
        else
        {
            if(this->findBluezSource() != NULL && this->findBluezSink() != NULL)
            {
                // bt source and sink exists
                m_btSourceReady = true;
                m_btSinkReady = true;
                DEBUG_T("Route microphone and speakers");
                routeAudio();
            }
            else
            {
                //no bt source or sink yet, let's wait until source and sink appears
                m_btSourceReady = false;
                m_btSinkReady = false;
                connect(this, SIGNAL(sourceAppeared(PADevice*)), this, SLOT(onSourceAppeared(PADevice*)));
                connect(this, SIGNAL(sinkAppeared(PADevice*)), this, SLOT(onSinkAppeared(PADevice*)));
                DEBUG_T("Audio not routed yet, wait for bt source and sinks");
            }
        }
    }
    else if (m_manager->voiceCallCount() <= 0)
    {
        DEBUG_T("no more ofono calls");
        if(m_audioRouted)
        {
            DEBUG_T("Unroute microphone and speakers");
            unrouteAudio();
        }
    }
}
