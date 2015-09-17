/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "farstreamchannel.h"

#include <TelepathyQt/Connection>
#include <TelepathyQt/Farstream/Channel>

#include <telepathy-farstream/channel.h>
#include <telepathy-farstream/content.h>

#include <farstream/fs-stream.h>
#include <farstream/fs-conference.h>
#include <farstream/fs-utils.h>

//#define VIDEO_SOURCE_ELEMENT "v4l2src"
//#define VIDEO_SOURCE_ELEMENT "autovideosrc"
//#define VIDEO_SOURCE_ELEMENT "videotestsrc"

//#define AUDIO_SOURCE_ELEMENT "autoaudiosrc"
//#define AUDIO_SOURCE_ELEMENT "audiotestsrc"
#define AUDIO_SOURCE_ELEMENT "pulsesrc"

//#define AUDIO_SINK_ELEMENT "autoaudiosink"
//#define AUDIO_SINK_ELEMENT "alsasink"
#define AUDIO_SINK_ELEMENT "pulsesink"

// when defined, incoming audio is teed there
//#define AUDIO_OUTPUT_FILE "/tmp/im-output.wav"

#define COLORSPACE_CONVERT_ELEMENT "ffmpegcolorspace"

#define SINK_GHOST_PAD_NAME "sink"
#define SRC_GHOST_PAD_NAME "src"

class LifetimeTracer {
private:
    const char *filename;
    int line;
    const char *function;
public:
    LifetimeTracer(const char *fn, int l, const char *f): filename(fn), line(l), function(f) { qDebug() << filename << ":" << line << ":" << " entering " << function; }
    ~LifetimeTracer() { qDebug() << filename << ":" << line << ":" << " leaving " << function; }
};
#define LIFETIME_TRACER() LifetimeTracer lifetime_tracer(__FILE__,__LINE__,__PRETTY_FUNCTION__)
#define TRACE() qDebug() << __FILE__ << ":" << __LINE__ << ": trace";

FarstreamChannel::FarstreamChannel(TfChannel *tfChannel, QObject *parent) :
    QObject(parent),
    mTfChannel(tfChannel),
    mState(Tp::MediaStreamStateDisconnected),
    mGstPipeline(0),
    mGstBus(0),
    mGstBusSource(0),
    mGstAudioInput(0),
    mGstAudioInputVolume(0),
    mGstAudioOutput(0),
    mGstAudioOutputVolume(0),
    mGstAudioOutputSink(0),
    mGstAudioOutputActualSink(0)
{
    LIFETIME_TRACER();

    if (!mTfChannel) {
        setError("Unable to create Farstream channel");
        return;
    }

    // connect to the glib-style signals in farstream channel
    mSHClosed = g_signal_connect(mTfChannel, "closed",
        G_CALLBACK(&FarstreamChannel::onClosed), this);
    mSHFsConferenceAdded = g_signal_connect(mTfChannel, "fs-conference-added",
        G_CALLBACK(&FarstreamChannel::onFsConferenceAdded), this);
    mSHFsConferenceRemoved = g_signal_connect(mTfChannel, "fs-conference-removed",
        G_CALLBACK(&FarstreamChannel::onFsConferenceRemoved), this);
    mSHContentAdded = g_signal_connect(mTfChannel, "content-added",
        G_CALLBACK(&FarstreamChannel::onContentAdded), this);
    mSHContentRemoved = g_signal_connect(mTfChannel, "content-removed",
        G_CALLBACK(&FarstreamChannel::onContentRemoved), this);
}

FarstreamChannel::~FarstreamChannel()
{
    LIFETIME_TRACER();

    deinitAudioOutput();
    deinitAudioInput();
    deinitGstreamer();

    if (mTfChannel) {
        if (g_signal_handler_is_connected(mTfChannel, mSHClosed)) {
            g_signal_handler_disconnect(mTfChannel, mSHClosed);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHFsConferenceAdded)) {
            g_signal_handler_disconnect(mTfChannel, mSHFsConferenceAdded);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHFsConferenceRemoved)) {
            g_signal_handler_disconnect(mTfChannel, mSHFsConferenceRemoved);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHContentAdded)) {
            g_signal_handler_disconnect(mTfChannel, mSHContentAdded);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHContentRemoved)) {
            g_signal_handler_disconnect(mTfChannel, mSHContentRemoved);
            mSHClosed = 0;
        }
        g_object_unref(mTfChannel);
        mTfChannel = 0;
    }
}

Tp::MediaStreamState FarstreamChannel::state() const
{
    return mState;
}

void FarstreamChannel::setState(Tp::MediaStreamState state)
{
    qDebug() << "FarstreamChannel::setState(" << state << ") current mState=" << mState;

    if (mState != state) {
        mState = state;
        emit stateChanged();
    }
}

void FarstreamChannel::setError(const QString &errorMessage)
{
    qDebug() << "ERROR: " << errorMessage;

    emit error(errorMessage);
}

void FarstreamChannel::init()
{
    LIFETIME_TRACER();
    initGstreamer();

    // surfaces and sinks are created now to avoid problems with threads
    if (mGstPipeline) {
        GstStateChangeReturn ret = gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            setError("GStreamer pipeline cannot be played");
            return;
        }
    }

    setState(Tp::MediaStreamStateConnecting);
}

void FarstreamChannel::initGstreamer()
{
    LIFETIME_TRACER();

    mGstPipeline = gst_pipeline_new(NULL);
    if (!mGstPipeline) {
        setError("Gstreamer pipeline could not be created");
        return;
    }

    mGstBus = gst_pipeline_get_bus(GST_PIPELINE(mGstPipeline));
    if (!mGstBus) {
        setError("Gstreamer bus could not be retrieved");
        return;
    }

    mGstBusSource = gst_bus_add_watch(mGstBus, (GstBusFunc) &FarstreamChannel::onBusWatch, this);
    if (!mGstBusSource) {
        setError("Gstreamer bus add watch failed");
        return;
    }
}

void FarstreamChannel::deinitGstreamer()
{
    LIFETIME_TRACER();

    foreach(FsElementAddedNotifier * notifier, mFsNotifiers) {
        fs_element_added_notifier_remove(notifier, GST_BIN(mGstPipeline));
        g_object_unref(notifier);
    }
    mFsNotifiers.clear();

    if (mGstBusSource) {
        g_source_remove(mGstBusSource);
        mGstBusSource = 0;
    }

    if (mGstBus) {
        gst_object_unref(mGstBus);
        mGstBus = 0;
    }

    if (mGstPipeline) {
        gst_element_set_state(mGstPipeline, GST_STATE_NULL);
        gst_object_unref(mGstPipeline);
        mGstPipeline = 0;
    }
}

static void setPhoneMediaRole(GstElement *element)
{
    GstStructure *props = gst_structure_from_string ("props,media.role=phone", NULL);
    g_object_set(element, "stream-properties", props, NULL);
    gst_structure_free(props);
}

static void releaseGhostPad(GstElement *bin, const char *name, GstElement *sink)
{
    LIFETIME_TRACER();

    // if trying to disconnect while connecting, this will not have been setup yet
    if (!bin)
        return;

    qDebug() << "Releasing ghost pad named " << name << " from bin " << gst_element_get_name(bin) << ", sink " << (sink ? gst_element_get_name(sink) : "<none>");
    if (bin) {
        GstPad *ghostPad = gst_element_get_static_pad(GST_ELEMENT(bin), name);
        if (GST_GHOST_PAD(ghostPad)) {
            GstPad *pad = gst_ghost_pad_get_target(GST_GHOST_PAD(ghostPad));
            if (pad) {
                gst_ghost_pad_set_target(GST_GHOST_PAD(ghostPad), NULL);
                if (sink) {
                    qDebug() << "Releasing request pad under ghost pad";
                    gst_element_release_request_pad(sink, pad);
                }
                gst_object_unref(pad);
            }
            gst_element_remove_pad(GST_ELEMENT(bin), ghostPad);
        }
    }
}

void FarstreamChannel::createGhostPad(GstElement *bin, GstPad *pad, const char *name)
{
    qDebug() << "Creating ghost pad named " << name << " for bin " << gst_element_get_name(bin);

    if (!pad) {
        setError("Failed to find pad on which to create ghost pad");
        return;
    }

    GstPad *ghost = gst_ghost_pad_new(name, pad);
    gst_object_unref(pad);
    if (!ghost) {
        setError("GStreamer ghost pad failed");
        return;
    }

    gboolean res = gst_element_add_pad(GST_ELEMENT(bin), ghost);
    if (!res) {
        setError("GStreamer add ghost pad failed");
        return;
    }
}

void FarstreamChannel::initAudioInput()
{
    LIFETIME_TRACER();

    if (mGstAudioInput) {
      qDebug() << "Audio input already initialized, doing nothing";
      return;
    }

    mGstAudioInput = gst_bin_new("audio-input-bin");
    if (!mGstAudioInput) {
        setError("GStreamer audio input bin could not be created");
        return;
    }
    gst_object_ref(mGstAudioInput);
    gst_object_sink(mGstAudioInput);

    GstElement *source = 0;
    source = addElementToBin(mGstAudioInput, source, AUDIO_SOURCE_ELEMENT);
    if (!source) {
        setError("GStreamer audio input source could not be created");
        return;
    }

    if (!strcmp(AUDIO_SOURCE_ELEMENT, "pulsesrc")) {
        g_object_set(source, "buffer-time", (gint64)20000, NULL);
        g_object_set(source, "latency-time", (gint64)10000, NULL);
        setPhoneMediaRole(source);
    }

    mGstAudioInputVolume = addElementToBin(mGstAudioInput, source, "volume");
    if (!mGstAudioInputVolume) {
        setError("GStreamer audio input volume could not be created");
    } else {
        source = mGstAudioInputVolume;
        gst_object_ref (mGstAudioInputVolume);
    }

    createGhostPad(mGstAudioInput, gst_element_get_static_pad(source, "src"), SRC_GHOST_PAD_NAME);
}

void FarstreamChannel::deinitAudioInput()
{
    LIFETIME_TRACER();

    if (!mGstAudioInput) {
      qDebug() << "Audio input not initialized, doing nothing";
      return;
    }

    gst_element_set_state(mGstAudioInput, GST_STATE_NULL);

    releaseGhostPad(mGstAudioInput, SRC_GHOST_PAD_NAME, NULL);

    if (mGstAudioInput) {
        gst_element_set_state(mGstAudioInput, GST_STATE_NULL);
        gst_object_unref(mGstAudioInput);
        mGstAudioInput = 0;
    }

    if (mGstAudioInputVolume) {
        gst_object_unref(mGstAudioInputVolume);
        mGstAudioInputVolume = 0;
    }
}

GstElement *FarstreamChannel::pushElement(GstElement *bin, GstElement *&last, const char *factory, bool optional, GstElement **copy, bool checkLink)
{
    if (copy)
        *copy = NULL;
    GstElement *e = addElementToBin(bin, last, factory, checkLink);
    if (!e) {
        if (optional) {
            qDebug() << "Failed to create or link optional element " << factory;
        }
        else {
            setError(QString("Failed to create or link element ") + factory);
        }
        return NULL;
    }
    last = e;
    if (copy) {
        gst_object_ref(e);
        *copy = e;
    }
    return e;
}

void FarstreamChannel::writeAudioToFile(GstElement *bin, GstElement *tee)
{
#ifdef AUDIO_OUTPUT_FILE
    GstPad *src, *sink;
    GstElement *source, *filesink;

    src = gst_element_get_request_pad(tee, "src%d");
    if (!src) {
        qWarning() << "Failed to get src pad from audio output tee";
        return;
    }
    source = gst_element_factory_make("queue", 0);
    if (!source) {
        qWarning() << "Failed to create queue element";
        return;
    }
    if (!gst_bin_add (GST_BIN (bin), source)) {
        qWarning() << "Failed to add queue to audio file bin";
        return;
    }
    sink = gst_element_get_static_pad (source, "sink");
    if (!sink) {
        qWarning() << "Failed to find static sink pad in queue";
        return;
    }
    if (gst_pad_link (src, sink) != GST_PAD_LINK_OK) {
        qWarning() << "Failed to link audio file tee to queue";
        return;
    }
    pushElement(bin, source, "queue", false, NULL, false);
    pushElement(bin, source, "wavenc", false, NULL, false);
    filesink = pushElement(bin, source, "filesink", false, NULL, false);
    if (filesink == NULL) {
        qWarning() << "Failed to push filesink";
        return;
    }
    g_object_set(filesink, "location", AUDIO_OUTPUT_FILE, NULL);
#else
    (void)bin;
    (void)tee;
#endif
}

void FarstreamChannel::initAudioOutput()
{
    LIFETIME_TRACER();

    if (mGstAudioOutput) {
      qDebug() << "Audio output already initialized, doing nothing";
      return;
    }

    mGstAudioOutputSink = NULL;
    mGstAudioOutput = gst_bin_new("audio-output-bin");
    if (!mGstAudioOutput) {
        setError("GStreamer audio output could not be created");
        return;
    }
    gst_object_ref(mGstAudioOutput);
    gst_object_sink(mGstAudioOutput);

    GstElement *source = 0;
    pushElement(mGstAudioOutput, source, "queue", false, &mGstAudioOutputSink, false);
    GstElement *tee = pushElement(mGstAudioOutput, source, "tee", false);

    if (strcmp(AUDIO_SINK_ELEMENT, "pulsesink")) {
        pushElement(mGstAudioOutput, source, "audioresample", true, NULL, false);
        pushElement(mGstAudioOutput, source, "volume", true, &mGstAudioOutputVolume, false);
    }
    else {
        mGstAudioOutputVolume = NULL;
    }
    pushElement(mGstAudioOutput, source, AUDIO_SINK_ELEMENT, false, &mGstAudioOutputActualSink ,false);

    if (!mGstAudioOutputSink) {
        mGstAudioOutputSink = mGstAudioOutputActualSink;
        gst_object_ref(mGstAudioOutputSink);
    }

    g_object_set(G_OBJECT(mGstAudioOutput), "async-handling", TRUE, NULL);
    createGhostPad(mGstAudioOutput, gst_element_get_static_pad(mGstAudioOutputSink, "sink"), SINK_GHOST_PAD_NAME);

    if (!strcmp(AUDIO_SINK_ELEMENT, "pulsesink")) {
        g_object_set(mGstAudioOutputActualSink, "buffer-time", (gint64)50000, NULL);
        g_object_set(mGstAudioOutputActualSink, "latency-time", (gint64)25000, NULL);
        setPhoneMediaRole(mGstAudioOutputActualSink);
    }

    writeAudioToFile(mGstAudioOutput, tee);
}

void FarstreamChannel::deinitAudioOutput()
{
    LIFETIME_TRACER();

    if (!mGstAudioOutput) {
      qDebug() << "Audio output not initialized, doing nothing";
      return;
    }

    gst_element_set_state(mGstAudioOutput, GST_STATE_NULL);

    releaseGhostPad(mGstAudioOutput, SINK_GHOST_PAD_NAME, NULL);

    if (mGstAudioOutput) {
        gst_element_set_state(mGstAudioOutput, GST_STATE_NULL);
        gst_object_unref(mGstAudioOutput);
        mGstAudioOutput = 0;
    }

    if (mGstAudioOutputVolume) {
        gst_object_unref(mGstAudioOutputVolume);
        mGstAudioOutputVolume = 0;
    }

    if (mGstAudioOutputSink) {
        gst_object_unref(mGstAudioOutputSink);
        mGstAudioOutputSink = 0;
    }

    if (mGstAudioOutputActualSink) {
        gst_object_unref(mGstAudioOutputActualSink);
        mGstAudioOutputActualSink = 0;
    }
}

void FarstreamChannel::onClosed(TfChannel *tfc, FarstreamChannel *self)
{
    Q_UNUSED(tfc);

    qDebug() << "FarstreamChannel::onClosed:";
    self->setState(Tp::MediaStreamStateDisconnected);
}

gboolean FarstreamChannel::onBusWatch(GstBus *bus, GstMessage *message, FarstreamChannel *self)
{
    Q_UNUSED(bus);

    if (!self->mTfChannel) {
        return TRUE;
    }

    const GstStructure *s = gst_message_get_structure(message);
    if (s == NULL) {
        goto error;
    }

    if (gst_structure_has_name (s, "farsight-send-codec-changed")) {

        const GValue *val = gst_structure_get_value(s, "codec");
        if (!val) {
            goto error;
        }

        FsCodec *codec = static_cast<FsCodec *> (g_value_get_boxed(val));
        if (!codec) {
            goto error;
        }

        val = gst_structure_get_value(s, "session");
        if (!val) {
            goto error;
        }

        FsSession *session = static_cast<FsSession *> (g_value_get_object(val));
        if (!session) {
            goto error;
        }

        FsMediaType type;
        g_object_get(session, "media-type", &type, NULL);

        gchar *codec_string = fs_codec_to_string(codec);
        qDebug() << "FarstreamChannel::onBusWatch: farsight-send-codec-changed "
                 << " type=" << type
                 << " codec=" << codec_string;
        g_free(codec_string);

    } else if (gst_structure_has_name(s, "farsight-recv-codecs-changed")) {

        const GValue *val = gst_structure_get_value(s, "codecs");
        if (!val) {
            goto error;
        }

        GList *codecs = static_cast<GList *> (g_value_get_boxed(val));
        if (!codecs) {
            goto error;
        }

        val = gst_structure_get_value(s, "stream");
        if (!val) {
            goto error;
        }

        FsStream *stream = static_cast<FsStream *> (g_value_get_object(val));
        if (!stream) {
            goto error;
        }

        FsSession *session = 0;
        g_object_get(stream, "session", &session, NULL);
        if (!session) {
            goto error;
        }

        FsMediaType type = FS_MEDIA_TYPE_LAST;
        g_object_get(session, "media-type", &type, NULL);

        qDebug() << "FarstreamChannel::onBusWatch: farsight-recv-codecs-changed "
                 << " type=" << type;
        GList *list;
        for (list = codecs; list != NULL; list = g_list_next(list)) {
            FsCodec *codec = static_cast<FsCodec *> (list->data);
            gchar *codec_string = fs_codec_to_string(codec);
            qDebug() << "       codec " << codec_string;
            g_free(codec_string);
        }

        g_object_unref(session);

    } else if (gst_structure_has_name(s, "farsight-new-active-candidate-pair")) {

        const GValue *val = gst_structure_get_value(s, "remote-candidate");
        if (!val) {
            goto error;
        }

        FsCandidate *remote_candidate = static_cast<FsCandidate *> (g_value_get_boxed(val));
        if (!remote_candidate) {
            goto error;
        }

        val = gst_structure_get_value(s, "local-candidate");
        if (!val) {
            goto error;
        }

        FsCandidate *local_candidate = static_cast<FsCandidate *> (g_value_get_boxed(val));
        if (!local_candidate) {
            goto error;
        }

        val = gst_structure_get_value(s, "stream");
        if (!val) {
            goto error;
        }

        FsStream *stream = static_cast<FsStream *> (g_value_get_object(val));
        if (!stream) {
            goto error;
        }

        FsSession *session = 0;
        g_object_get(stream, "session", &session, NULL);
        if (!session) {
            goto error;
        }

        FsMediaType type = FS_MEDIA_TYPE_LAST;
        g_object_get(session, "media-type", &type, NULL);

        qDebug() << "FarstreamChannel::onBusWatch: farsight-new-active-candidate-pair ";

        if (remote_candidate) {
            qDebug() << "   remote candidate mediatype=" << type
                     << " foundation=" << remote_candidate->foundation << " id=" << remote_candidate->component_id
                     << " IP=" << remote_candidate->ip << ":" << remote_candidate->port
                     << " BaseIP=" << remote_candidate->base_ip << ":" << remote_candidate->base_port
                     << " proto=" << remote_candidate->proto
                     << " type=" << remote_candidate->type;
        }

        if (local_candidate) {
            qDebug() << "   local candidate mediatype=" << type
                     << " foundation=" << local_candidate->foundation << " id=" << local_candidate->component_id
                     << " IP=" << local_candidate->ip << ":" << local_candidate->port
                     << " BaseIP=" << local_candidate->base_ip << ":" << local_candidate->base_port
                     << " proto=" << local_candidate->proto
                     << " type=" << local_candidate->type;
        }

        g_object_unref(session);
    }

error:

    tf_channel_bus_message(self->mTfChannel, message);
    return TRUE;
}

void FarstreamChannel::setMute(bool mute)
{
    qDebug() << "FarstreamChannel::setMute: mute=" << mute;

    if (!mGstAudioInputVolume) {
        return;
    }

    g_object_set(G_OBJECT(mGstAudioInputVolume), "mute", mute, NULL);
}

bool FarstreamChannel::mute() const
{
    if (!mGstAudioInputVolume) {
        return false;
    }

    gboolean mute = FALSE;
    g_object_get (G_OBJECT(mGstAudioInputVolume), "mute", &mute, NULL);

    return mute;
}

void FarstreamChannel::setInputVolume(double volume)
{
    qDebug() << "FarstreamChannel::setInputVolume: volume=" << volume;

    if (!mGstAudioInputVolume) {
        return;
    }

    GParamSpec *pspec;
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(mGstAudioInputVolume), "volume");
    if (!pspec) {
        return;
    }

    GParamSpecDouble *pspec_double;
    pspec_double = G_PARAM_SPEC_DOUBLE(pspec);
    if (!pspec_double) {
        return;
    }

    volume = CLAMP(volume, pspec_double->minimum, pspec_double->maximum);
    g_object_set(G_OBJECT(mGstAudioInputVolume), "volume", volume, NULL);
}

double FarstreamChannel::inputVolume() const
{
    if (!mGstAudioInputVolume) {
        return 0.0;
    }

    double volume;
    g_object_get(G_OBJECT(mGstAudioInputVolume), "volume", &volume, NULL);

    return volume;
}

void FarstreamChannel::setVolume(double volume)
{
    qDebug() << "FarstreamChannel::setVolume: volume=" << volume;

    GstElement *volumeElement = mGstAudioOutputVolume ? mGstAudioOutputVolume : mGstAudioOutputActualSink;
    if (!volumeElement) {
        return;
    }

    GParamSpec *pspec;
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(volumeElement), "volume");
    if (!pspec) {
        return;
    }

    GParamSpecDouble *pspec_double;
    pspec_double = G_PARAM_SPEC_DOUBLE(pspec);
    if (!pspec_double) {
        return;
    }

    volume = CLAMP(volume, pspec_double->minimum, pspec_double->maximum);
    g_object_set(G_OBJECT(volumeElement), "volume", volume, NULL);
}

double FarstreamChannel::volume() const
{
    GstElement *volumeElement = mGstAudioOutputVolume ? mGstAudioOutputVolume : mGstAudioOutputActualSink;
    if (!volumeElement) {
        return 0.0;
    }

    double volume;
    g_object_get(G_OBJECT(volumeElement), "volume", &volume, NULL);

    return volume;
}

void FarstreamChannel::stop()
{
  if (mGstPipeline) {
    gst_element_set_state(mGstPipeline, GST_STATE_NULL);
  }
}

GstElement *FarstreamChannel::addElementToBin(GstElement *bin, GstElement *src, const char *factoryName, bool checkLink)
{
    qDebug() << "FarstreamChannel::addElementToBin: bin=" << bin << " src=" << src << " factoryName=" << factoryName;

    GstBin *binobj = GST_BIN(bin);
    if (!binobj) {
        setError(QLatin1String("Element factory not found ") + factoryName);
        return 0;
    }

    GstElement *ret;
    if ((ret = gst_element_factory_make(factoryName, 0)) == 0) {
        setError(QLatin1String("Element factory not found ") + factoryName);
        return 0;
    }
    
    return addAndLink(binobj, src, ret, checkLink);
}

GstElement *FarstreamChannel::addAndLink(GstBin *binobj, GstElement *src, GstElement *ret, bool checkLink)
{
    gboolean res;

    qDebug() << "FarstreamChannel::addAndLink: binobj="
	     << gst_element_get_name(GST_ELEMENT(binobj)) 
	     << " src="
	     << (src ? gst_element_get_name(src) : "(NULL)")
             << " dst="
	     << gst_element_get_name(ret);

    if (!gst_bin_add(binobj, ret)) {
        setError(QLatin1String("Could not add to bin "));
        gst_object_unref(ret);
        return 0;
    }

    if (!src) {
        return ret;
    }

    if (checkLink) {
      res = gst_element_link(src, ret);
    }
    else {
      res = gst_element_link_pads_full(src, NULL, ret, NULL, GST_PAD_LINK_CHECK_NOTHING);
    }
    if (!res) {
        setError(QLatin1String("Failed to link "));
        gst_bin_remove(binobj, ret);
        return 0;
    }

    return ret;
}

void FarstreamChannel::onFsConferenceAdded(TfChannel *tfc, FsConference * conf, FarstreamChannel *self)
{
    Q_UNUSED(tfc);
    Q_ASSERT(conf);

    qDebug() << "FarstreamChannel::onFsConferenceAdded: tfc=" << tfc << " conf=" << conf << " self=" << self;

    if (!self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    /* Add notifier to set the various element properties as needed */
    GKeyFile *keyfile = fs_utils_get_default_element_properties(GST_ELEMENT(conf));
    if (keyfile != NULL)
    {
        qDebug() << "Loaded default codecs for " << GST_ELEMENT_NAME(conf);
        FsElementAddedNotifier *notifier = fs_element_added_notifier_new();
        fs_element_added_notifier_set_properties_from_keyfile(notifier, keyfile);
        fs_element_added_notifier_add (notifier, GST_BIN (self->mGstPipeline));
        self->mFsNotifiers.append(notifier);
    }

    // add conference to the pipeline
    gboolean res = gst_bin_add(GST_BIN(self->mGstPipeline), GST_ELEMENT(conf));
    if (!res) {
        self->setError("GStreamer farsight conference could not be added to the bin");
        return;
    }

    GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(conf), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        self->setError("GStreamer farsight conference cannot be played");
        return;
    }
}

void FarstreamChannel::onFsConferenceRemoved(TfChannel *tfc, FsConference * conf, FarstreamChannel *self)
{
    Q_UNUSED(tfc);
    Q_ASSERT(conf);

    qDebug() << "FarstreamChannel::onFsConferenceRemoved: tfc=" << tfc << " conf=" << conf << " self=" << self;

    if (!self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(conf), GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        self->setError("GStreamer farsight conference cannot be set to null");
        return;
    }

    if (self->mGstPipeline) {
        // remove conference to the pipeline
        gboolean res = gst_bin_remove(GST_BIN(self->mGstPipeline), GST_ELEMENT(conf));
        if (!res) {
            self->setError("GStreamer farsight conference could not be added to the bin");
            return;
        }
    }
}

static const char *get_media_type_string(guint type)
{
  switch (type) {
    case TP_MEDIA_STREAM_TYPE_AUDIO:
      return "audio";
    case TP_MEDIA_STREAM_TYPE_VIDEO:
      return "video";
    default:
      Q_ASSERT(false);
      return "unknown";
  }
}

void FarstreamChannel::addBin(GstElement *bin)
{
    if (!bin)
        return;

    gboolean res = gst_bin_add(GST_BIN(mGstPipeline), bin);
    if (!res) {
        setError("GStreamer could not add bin to the pipeline");
        return;
    }

    gst_element_set_locked_state (bin, TRUE);
}

void FarstreamChannel::onContentAdded(TfChannel *tfc, TfContent * content, FarstreamChannel *self)
{
    Q_UNUSED(tfc);

    LIFETIME_TRACER();

    /*
    * Tells the application that a content has been added. In the callback for
    * this signal, the application should set its preferred codecs, and hook
    * up to any signal from #TfContent it cares about. Special care should be
    * made to connect #TfContent::src-pad-added as well
    * as the #TfContent::start-sending and #TfContent::stop-sending signals.
    */

    if (!self || !self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    if (!content) {
        self->setError("Invalid content received");
        return;
    }

    g_signal_connect(content, "start-sending",
                     G_CALLBACK(&FarstreamChannel::onStartSending), self);
    g_signal_connect(content, "stop-sending",
                     G_CALLBACK(&FarstreamChannel::onStopSending), self);
    g_signal_connect(content, "src-pad-added",
                     G_CALLBACK(&FarstreamChannel::onSrcPadAddedContent), self);

    guint media_type;
    g_object_get(content, "media-type", &media_type, NULL);
    qDebug() << "FarstreamChannel::onContentAdded: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    /*
    FsSession *session;
    g_object_get(content, "fs-session", &session, NULL);
    if (session) {

        FsCodec *codec;
        g_object_get(session, "current-send-codec", &codec, NULL);
        if (codec) {
            qDebug() << "FarstreamChannel::onStreamCreated: current send codec=" << fs_codec_to_string(codec);
        }

        FsStream *fs_stream = 0;
        g_object_get(content, "farsight-stream", &fs_stream, NULL);
        if (fs_stream) {
            GList *codecs;
            g_object_get(fs_stream, "current-recv-codecs", &codecs, NULL);
            if (codecs) {
                qDebug() << " current received codecs: ";
                GList *list;
                for (list = codecs; list != NULL; list = g_list_next (list)) {
                    FsCodec *codec = static_cast<FsCodec *> (list->data);
                    qDebug() << "       codec " << fs_codec_to_string(codec);
                }

                fs_codec_list_destroy(codecs);
            }
        }
    }*/

    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Got audio content, adding audio bins";
        self->initAudioInput();
        self->initAudioOutput();
        self->addBin(self->mGstAudioInput);
    } else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
        qDebug() << "Got video content, ignoring.";
    } else {
        Q_ASSERT(false);
        return;
    }
}

void FarstreamChannel::removeBin(GstElement *bin, bool isSink)
{
    if (!bin)
        return;

    gst_element_set_locked_state(bin, TRUE);

    if (gst_element_set_state(bin, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
        setError("Failed to stop bin");
        return;
    }

    TRACE();
    GstPad *pad = gst_element_get_static_pad(bin, isSink ? SINK_GHOST_PAD_NAME : SRC_GHOST_PAD_NAME);
    if (!pad) {
        setError("GStreamer get sink element source pad failed");
        return;
    }

    TRACE();
    GstPad *peer = gst_pad_get_peer (pad);
    if (!peer) {
        if (isSink) {
          qDebug() << "Pad has no peer, but it's from a sink which may not have been added, done";
        }
        else {
          setError("Pad has no peer");
        }
    }
    else {
        bool resUnlink = GST_PAD_IS_SRC (pad) ? gst_pad_unlink (pad, peer) : gst_pad_unlink(peer, pad);
        if (!resUnlink) {
            setError("GStreamer could not unlink output bin pad");
            return;
        }
    }

    TRACE();
    gboolean res = gst_bin_remove(GST_BIN(mGstPipeline), bin);
    if (!res) {
        setError("GStreamer could not remove bin from the pipeline");
        return;
    }
    TRACE();
}

void FarstreamChannel::onContentRemoved(TfChannel *tfc, TfContent * content, FarstreamChannel *self)
{
    Q_UNUSED(tfc);
    Q_UNUSED(content);
    Q_UNUSED(self);

    LIFETIME_TRACER();

    if (!self || !self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    if (!content) {
        self->setError("Invalid content received");
        return;
    }

    guint media_type;
    g_object_get(content, "media-type", &media_type, NULL);
    qDebug() << "FarstreamChannel::onContentRemoved: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Audio content removed";
        self->removeBin(self->mGstAudioInput);
        self->removeBin(self->mGstAudioOutput, true);
    } else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
    } else {
        Q_ASSERT(false);
        return;
    }
}

bool FarstreamChannel::onStartSending(TfContent *content, FarstreamChannel *self)
{
    LIFETIME_TRACER();

    if (!self || !self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return false;
    }

    if (!content) {
        self->setError("Invalid content received");
        return false;
    }

    guint media_type;
    GstPad *sink;
    g_object_get(content, "media-type", &media_type, "sink-pad", &sink, NULL);
    if (!sink) {
        self->setError("GStreamer cannot get sink pad from content");
        return false;
    }
    qDebug() << "FarstreamChannel::onStartSending: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    GstElement *sourceElement = 0;
    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Got audio sink, initializing audio input";
        if (!self->mGstAudioInput) {
            self->initAudioInput();
        }
        sourceElement = self->mGstAudioInput;
    } else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
    } else {
        Q_ASSERT(false);
        return false;
    }

    if (!sourceElement) {
        self->setError("GStreamer source element not found");
        return false;
    }

    GstPad *pad = gst_element_get_static_pad(sourceElement, "src");
    if (!pad) {
        self->setError("GStreamer get source element source pad failed");
        return false;
    }

    GstPadLinkReturn resLink = gst_pad_link(pad, sink);
    gst_object_unref(pad);
    gst_object_unref(sink);
    if (resLink != GST_PAD_LINK_OK && resLink != GST_PAD_LINK_WAS_LINKED) {
        self->setError("GStreamer could not link input source pad to sink");
        return false;
    }

    gst_element_set_locked_state(sourceElement, FALSE);

    if (!gst_element_sync_state_with_parent(sourceElement)) {
        self->setError("GStreamer input state could not be synced with parent");
        return false;
    }

    //GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(self->mGstPipeline), GST_DEBUG_GRAPH_SHOW_ALL, "impipeline1");

    return true;
}

void FarstreamChannel::onStopSending(TfContent *content, FarstreamChannel *self)
{
    LIFETIME_TRACER();

    guint media_type;
    GstPad *sink;
    g_object_get(content, "media-type", &media_type, "sink-pad", &sink, NULL);
    if (!sink) {
        self->setError("GStreamer cannot get sink pad from content");
        return;
    }
    qDebug() << "FarstreamChannel::onStopSending: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    TRACE();
    GstElement *sourceElement = 0;
    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Got audio sink";
        if (!self->mGstAudioInput) {
            qDebug() << "Audio input is not initialized";
            return;
        }
        sourceElement = self->mGstAudioInput;
    } else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
    } else {
        Q_ASSERT(false);
    }

    TRACE();
    if (!sourceElement) {
        self->setError("GStreamer source element not found");
        return;
    }

    gst_element_set_locked_state(sourceElement, TRUE);

    TRACE();
    if (gst_element_set_state(sourceElement, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
        self->setError("Failed to stop bin");
        return;
    }

    TRACE();
    GstPad *pad = gst_element_get_static_pad(sourceElement, "src");
    if (!pad) {
        self->setError("GStreamer get source element source pad failed");
        return;
    }

    TRACE();
    bool resUnlink = gst_pad_unlink(pad, sink);
    if (!resUnlink) {
        self->setError("GStreamer could not unlink input source pad from sink");
        return;
    }

    TRACE();
}

void FarstreamChannel::onSrcPadAddedContent(TfContent *content, uint handle, FsStream *stream, GstPad *src, FsCodec *codec, FarstreamChannel *self)
{
    Q_UNUSED(content);
    Q_UNUSED(handle);
    Q_UNUSED(codec);
    Q_ASSERT(stream);

    LIFETIME_TRACER();

    guint media_type;
    g_object_get(content, "media-type", &media_type, NULL);

    qDebug() << "FarstreamChannel::onSrcPadAddedContent: stream=" << stream << " type=" << media_type << " (" << get_media_type_string(media_type) << ")" << "pad = " << src;

    GstElement *bin = 0;
    GstPad *pad = 0;

    switch (media_type) {
    case TP_MEDIA_STREAM_TYPE_AUDIO:
        bin = self->mGstAudioOutput;
        break;
    case TP_MEDIA_STREAM_TYPE_VIDEO:
        break;
    default:
        Q_ASSERT(false);
    }

    // Add sink bin the first time we get incoming data
    if (!gst_object_has_ancestor(GST_OBJECT(bin), GST_OBJECT(self->mGstPipeline))) {
        self->addBin(bin);
    }

    pad = gst_element_get_static_pad(bin, SINK_GHOST_PAD_NAME);
    if (!pad) {
        self->setError("Could not find ghost sink pad in bin");
        return;
    }

    /* We can get src-pad-added multiple times without being aware the stream
       might have stopped in the meantime */
    if (gst_pad_is_linked(pad)) {
        gst_element_set_locked_state(bin, TRUE);
        gst_element_set_state (bin, GST_STATE_READY);
        gst_pad_unlink (gst_pad_get_peer(pad), pad);
    }

    GstPadLinkReturn resLink = gst_pad_link(src, pad);
    if (resLink != GST_PAD_LINK_OK && resLink != GST_PAD_LINK_WAS_LINKED) {
        //tf_content_error(content, TP_MEDIA_STREAM_ERROR_MEDIA_ERROR, "Could not link sink");
        self->setError("GStreamer could not link sink pad to source");
        return;
    }

    gst_element_set_locked_state (bin, FALSE);
    if (gst_element_set_state (bin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        self->setError("GStreamer could not set output bin state to PLAYING");
        return;
    }

    self->setState(Tp::MediaStreamStateConnected);

    if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
    }

    // todo If no sink could be linked, try to add fakesink to prevent the whole call

    //GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(self->mGstPipeline), GST_DEBUG_GRAPH_SHOW_ALL, "impipeline2");
}

