/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef FARSIGHTCHANNEL_H
#define FARSIGHTCHANNEL_H

#include <QObject>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>
#include <TelepathyQt/Farstream/Channel>
#include <telepathy-farstream/content.h>

#undef signals // Collides with GTK symbols

#include <gio/gio.h>
#include <glib-object.h>
#include <farstream/fs-conference.h>
#include <farstream/fs-element-added-notifier.h>
#include <farstream/fs-stream.h>

class FarstreamChannel : public QObject
{
    Q_OBJECT

public:
    explicit FarstreamChannel(TfChannel *tfChannel, QObject *parent = 0);
    ~FarstreamChannel();

    /// global initialization
    void init();
    Tp::MediaStreamState state() const;

    void setMute(bool mute);
    bool mute() const;
    void setInputVolume(double volume);
    double inputVolume() const;
    void setVolume(double volume);
    double volume() const;

    void stop();

Q_SIGNALS:
    void stateChanged();
    void error(const QString &errorMessage);

private:    
    TfChannel *mTfChannel;
    Tp::MediaStreamState mState;

    GstElement *mGstPipeline;
    QList<FsElementAddedNotifier *> mFsNotifiers;
    GstBus *mGstBus;
    uint mGstBusSource;
    GstElement *mGstAudioInput;
    GstElement *mGstAudioInputVolume;
    GstElement *mGstAudioOutput;
    GstElement *mGstAudioOutputVolume;
    GstElement *mGstAudioOutputSink;
    GstElement *mGstAudioOutputActualSink;

    // glib signal handlers
    gulong mSHClosed;
    gulong mSHFsConferenceAdded;
    gulong mSHFsConferenceRemoved;
    gulong mSHContentAdded;
    gulong mSHContentRemoved;

    void setState(Tp::MediaStreamState state);
    void setError(const QString &errorMessage);

    /// initialize the common gstreamer stuff like bus, pipeline
    void initGstreamer();
    void deinitGstreamer();
    /// initialize audio input (mic to local audio stream)
    void initAudioInput();
    void deinitAudioInput();
    /// initialize audio output (remote audio stream to speaker)
    void initAudioOutput();
    void deinitAudioOutput();

    // glib style signal handlers for GStream
    static void onClosed(TfChannel *tfc, FarstreamChannel *self);
    static gboolean onBusWatch(GstBus *bus, GstMessage *message, FarstreamChannel *self);

    // signals related with call channel stuff
    static void onFsConferenceAdded(TfChannel *tfc, FsConference * conf, FarstreamChannel *self);
    static void onFsConferenceRemoved(TfChannel *tfc, FsConference * conf, FarstreamChannel *self);
    static void onContentAdded(TfChannel *tfc, TfContent * content, FarstreamChannel *self);
    static void onContentRemoved(TfChannel *tfc, TfContent * content, FarstreamChannel *self);
    static bool onStartSending(TfContent *tfc, FarstreamChannel *self);
    static void onStopSending(TfContent *tfc, FarstreamChannel *self);
    static void onSrcPadAddedContent(TfContent *content, uint handle, FsStream *stream, GstPad *src, FsCodec *codec, FarstreamChannel *self);

    GstElement *addElementToBin(GstElement *bin, GstElement *src, const char *factoryName, bool checkLink = true);
    GstElement *addAndLink(GstBin *bin, GstElement *src, GstElement * target, bool checkLink = true);

    GstElement *pushElement(GstElement *bin, GstElement *&last, const char *factory, bool optional = false, GstElement **copy = NULL, bool checkLink = true);
    void writeAudioToFile(GstElement *bin, GstElement *tee);

    void createGhostPad(GstElement *bin, GstPad *pad, const char *name);
    void addBin(GstElement*);
    void removeBin(GstElement *bin, bool isSink = false);
};

#endif // FARSIGHTCHANNEL_H
