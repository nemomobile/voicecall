/*
 * hfdialer - Hands Free Voice Call Manager
 * Copyright (c) 2012, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef PACONTROL_H
#define PACONTROL_H
#include <pulse/context.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <QObject>

#include <voicecallmanagerinterface.h>

class PADevice {
public:
    QString name;
    int index;
    QString description;
};

class PAModule {
public:
    QString name;
    int index;
    QString argument;
};

enum PAStatus {
    SUCCESS = 0,
    ERROR = 1,
};

class PAControl : public QObject
{
    Q_OBJECT;
public:
    ~PAControl();

    static PAControl* instance(VoiceCallManagerInterface *manager);

    void reconnect();
    PADevice* findBluezSource();
    PADevice* findBluezSink();
    PADevice* findAlsaSource(QString alsasource);
    PADevice* findAlsaSink(QString alsasink);
    PAModule* findModule(QString name, QString pattern);
    QList<PAModule*> getAllModules();
    void routeSourceWithSink(PADevice *source, PADevice *sink);

    void unloadModule(PAModule* module);
    void toggleMuteSource(PADevice *source, bool isMute);

    PAStatus getStatus();
    void setErrorMsg(QString msg);
    QString getErrorMsg();

    void setState(bool state);
    void addRef();
    void release();

    void addSource(PADevice* device);
    void addSink(PADevice* device);
    void addModule(PAModule* module);

    QList<PADevice*> sourceList;
    QList<PADevice*> sinkList;
    QList<PAModule*> moduleList;

public Q_SLOTS:
    void routeAudio();
    void unrouteAudio();

Q_SIGNALS:
    void sourceAppeared(PADevice* device);
    void sinkAppeared(PADevice* device);
    void moduleAppeared(PAModule* device);
    void paFailed();

private Q_SLOTS:
    void onSourceAppeared(PADevice* device);
    void onSinkAppeared(PADevice* device);
    void onCallsChanged();

private:
    PAControl(VoiceCallManagerInterface *manager, QObject *parent = 0);

    pa_operation *pa_op;
    pa_context *pa_ctx;
    PAStatus status;
    QString errorMsg;

    void paInit();
    void paCleanup();

    VoiceCallManagerInterface *m_manager;

    bool m_paState;
    int  m_refCounter;
    bool m_connected;
    bool m_audioRouted;
    bool m_btSourceReady;
    bool m_btSinkReady;
};

#endif // PACONTROL_H

/* Local Variables:      */
/* mode:c++              */
/* c-basic-offset:4      */
/* indent-tabs-mode: nil */
/* End:                  */
