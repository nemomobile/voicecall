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

protected Q_SLOTS:
    void onChannelReady(Tp::PendingOperation *op);
    void onChannelPropertyChanged(const QString &property);
    void onChannelInvalidated(Tp::DBusProxy*,const QString &errorName, const QString &errorMessage);

    void onChannelCallStateChanged(Tp::CallState state);
    void onChannelCallContentAdded(Tp::CallContentPtr content);
    void onChannelCallContentRemoved(Tp::CallContentPtr content, Tp::CallStateReason reason);
    void onChannelCallLocalHoldStateChanged(Tp::LocalHoldState state,Tp::LocalHoldStateReason reason);

    void onStreamAdded(const Tp::StreamedMediaStreamPtr &stream);
    void onStreamRemoved(const Tp::StreamedMediaStreamPtr &stream);
    void onStreamStateChanged(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamState state);
    void onStreamError(const Tp::StreamedMediaStreamPtr &stream, Tp::MediaStreamError errorCode, const QString &errorMessage);

    void onAcceptCallFinished(Tp::PendingOperation *op);
    void onHangupCallFinished(Tp::PendingOperation *op);

    void onCallStateChanged(uint contact, uint state);

private:
    class TelepathyHandlerPrivate *d_ptr;

    Q_DISABLE_COPY(TelepathyHandler)
    Q_DECLARE_PRIVATE(TelepathyHandler)
};

#endif // TELEPATHYHANDLER_H
