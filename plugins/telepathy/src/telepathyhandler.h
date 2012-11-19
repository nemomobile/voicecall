#ifndef TELEPATHYHANDLER_H
#define TELEPATHYHANDLER_H

#include <abstractvoicecallhandler.h>

#include <TelepathyQt/Channel>
#include <TelepathyQt/CallContent>

class TelepathyProvider;

class TelepathyHandler : public AbstractVoiceCallHandler
{
    Q_OBJECT

public:
    explicit TelepathyHandler(const QString &id, Tp::ChannelPtr channel, const QDateTime &userActionTime, TelepathyProvider *provider = 0);
            ~TelepathyHandler();

    /*** AbstractVoiceCallHandler Implementation ***/
    AbstractVoiceCallProvider* provider() const;
    QString handlerId() const;
    QString lineId() const;
    QDateTime startedAt() const;
    int duration() const;
    bool isMultiparty() const;
    bool isEmergency() const;

    VoiceCallStatus status() const;
    QString statusText() const;

    /*** TelepathyHandler Implementation ***/
    Tp::Channel channel() const;

Q_SIGNALS:
    /*** TelepathyHandler Implementation ***/
    void error(const QString &errorMessage);
    void invalidated(const QString &errorName, const QString &errorMessage);

public Q_SLOTS:
    /*** AbstractVoiceCallHandler Implementation ***/
    void answer();
    void hangup();
    void deflect(const QString &target);

    void sendDtmf(const QString &tones);

protected Q_SLOTS:
    void onStatusChanged();

    // TODO: Remove when tp-ring updated to call channel interface.
    // StreamedMediaChannel Interface Handling
    void onStreamedMediaChannelReady(Tp::PendingOperation *op);
    void onStreamedMediaChannelInvalidated(Tp::DBusProxy*,const QString &errorName, const QString &errorMessage);

    void onStreamedMediaChannelStreamAdded(const Tp::StreamedMediaStreamPtr &stream);
    void onStreamedMediaChannelStreamRemoved(const Tp::StreamedMediaStreamPtr &stream);

    void onStreamedMediaChannelStreamStateChanged(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamState state);
    void onStreamedMediaChannelStreamError(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamError errorCode, const QString &errorMessage);

    void onStreamedMediaChannelAcceptCallFinished(Tp::PendingOperation *op);
    void onStreamedMediaChannelHangupCallFinished(Tp::PendingOperation *op);

    // StreamedMediaChannel Group Interface Handling
    void onStreamedMediaChannelGroupMembersChanged(QString message, Tp::UIntList added, Tp::UIntList removed, Tp::UIntList localPending, Tp::UIntList remotePending, uint actor, uint reason);

    // CallChannel Interface Handling
    void onCallChannelChannelReady(Tp::PendingOperation *op);
    void onCallChannelChannelInvalidated(Tp::DBusProxy*,const QString &errorName, const QString &errorMessage);

    void onCallChannelCallStateChanged(Tp::CallState state);

    void onCallChannelCallContentAdded(Tp::CallContentPtr content);
    void onCallChannelCallContentRemoved(Tp::CallContentPtr content, Tp::CallStateReason reason);
    void onCallChannelCallLocalHoldStateChanged(Tp::LocalHoldState state,Tp::LocalHoldStateReason reason);

    void onCallChannelAcceptCallFinished(Tp::PendingOperation *op);
    void onCallChannelHangupCallFinished(Tp::PendingOperation *op);

    // Telepathy Farstream Interface Handling
    void onFarstreamCreateChannelFinished(Tp::PendingOperation *op);

protected:
    void timerEvent(QTimerEvent *event);

private:
    class TelepathyHandlerPrivate *d_ptr;

    Q_DISABLE_COPY(TelepathyHandler)
    Q_DECLARE_PRIVATE(TelepathyHandler)
};

#endif // TELEPATHYHANDLER_H
