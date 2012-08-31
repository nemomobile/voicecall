#include "common.h"

#include "telepathyhandler.h"
#include "telepathyprovider.h"

#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/ChannelClassSpec>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/PendingChannelRequest>

class TelepathyProviderPrivate
{
    Q_DECLARE_PUBLIC(TelepathyProvider)

public:
    TelepathyProviderPrivate(Tp::AccountPtr a, VoiceCallManagerInterface *m, TelepathyProvider *q)
        : q_ptr(q), manager(m), account(a),
          errorString(QString::null),
          tpClientHandler(NULL), tpClientRegistrar(NULL), tpPendingChannel(NULL)
    { /* ... */ }

    TelepathyProvider           *q_ptr;
    VoiceCallManagerInterface   *manager;

    Tp::AccountPtr               account;

    QString                      errorString;

    QHash<QString,AbstractVoiceCallHandler*> voiceCalls;

    Tp::AbstractClientPtr    tpClientHandler;
    Tp::ClientRegistrarPtr   tpClientRegistrar;

    Tp::PendingChannel      *tpPendingChannel;

    static const Tp::ChannelClassSpecList CHANNEL_SPECS;
};

const Tp::ChannelClassSpecList TelepathyProviderPrivate::CHANNEL_SPECS =
        (Tp::ChannelClassSpecList()
         << Tp::ChannelClassSpec::audioCall()
         << Tp::ChannelClassSpec::streamedMediaCall()
         << Tp::ChannelClassSpec::unnamedStreamedMediaCall()
         << Tp::ChannelClassSpec::streamedMediaAudioCall()
         << Tp::ChannelClassSpec::unnamedStreamedMediaCall());

TelepathyProvider::TelepathyProvider(Tp::AccountPtr account, VoiceCallManagerInterface *manager, QObject *parent)
    : AbstractVoiceCallProvider(parent),
      Tp::AbstractClientHandler(TelepathyProviderPrivate::CHANNEL_SPECS),
      d_ptr(new TelepathyProviderPrivate(account, manager, this))
{
    TRACE
    Q_D(TelepathyProvider);

    d->tpClientHandler = Tp::AbstractClientPtr(this);
    d->tpClientRegistrar = Tp::ClientRegistrar::create();

    if(!d->tpClientRegistrar->registerClient(d->tpClientHandler, "voicecall", true))
    {
        WARNING_T("Failed to register telepathy client!");
        d->errorString = "Failed to register telepathy client!";
        emit this->error(d->errorString);
    }
}

TelepathyProvider::~TelepathyProvider()
{
    TRACE
    Q_D(TelepathyProvider);
    d->tpClientRegistrar->unregisterClient(d->tpClientHandler);
    delete d;
}

QString TelepathyProvider::errorString() const
{
    TRACE
    Q_D(const TelepathyProvider);
    return d->errorString;
}

QString TelepathyProvider::providerId() const
{
    TRACE
    Q_D(const TelepathyProvider);
    return QString("telepathy-") + d->account->uniqueIdentifier();
}

QString TelepathyProvider::providerType() const
{
    TRACE
    return "cellular";
}

QList<AbstractVoiceCallHandler*> TelepathyProvider::voiceCalls() const
{
    TRACE
    Q_D(const TelepathyProvider);
    return d->voiceCalls.values();
}

bool TelepathyProvider::dial(const QString &msisdn)
{
    TRACE
    Q_D(TelepathyProvider);
    if(d->tpPendingChannel)
    {
        d->errorString = "Can't initiate a call when one is pending!";
        WARNING_T(d->errorString);
        emit this->error(d->errorString);
        return false;
    }

    d->tpPendingChannel = d->account->ensureAndHandleStreamedMediaAudioCall(msisdn);
    QObject::connect(d->tpPendingChannel,
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onDialFinished(Tp::PendingOperation*)));
    return true;
}

void TelepathyProvider::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                       const Tp::AccountPtr &account,
                                       const Tp::ConnectionPtr &connection,
                                       const QList<Tp::ChannelPtr> &channels,
                                       const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                       const QDateTime &userActionTime,
                                       const HandlerInfo &handlerInfo)
{
    TRACE
    Q_UNUSED(account)
    Q_UNUSED(account)
    Q_UNUSED(connection)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(handlerInfo)

    DEBUG_T(QString("Found %1 channel/s.").arg(channels.size()));

    foreach(Tp::ChannelPtr ch, channels)
    {
        this->createHandler(ch, userActionTime);
    }

    emit this->voiceCallsChanged();
    context->setFinished();
}

void TelepathyProvider::createHandler(Tp::ChannelPtr ch, const QDateTime &userActionTime)
{
    TRACE
    Q_D(TelepathyProvider);
    DEBUG_T(QString("\tProcessing channel: %1").arg(ch->objectPath()));
    TelepathyHandler *handler = new TelepathyHandler(d->manager->generateHandlerId(), Tp::StreamedMediaChannelPtr::staticCast(ch), userActionTime, this);
    d->voiceCalls.insert(handler->handlerId(), handler);

    QObject::connect(handler, SIGNAL(error(QString)), SIGNAL(error(QString)));

    QObject::connect(handler,
                     SIGNAL(invalidated(QString,QString)),
                     SLOT(onHandlerInvalidated(QString,QString)));

    emit this->voiceCallAdded(handler);
}

void TelepathyProvider::onDialFinished(Tp::PendingOperation *op)
{
    TRACE
    Q_D(TelepathyProvider);
    if(op->isError())
    {
        WARNING_T(QString("Operation failed: ") + op->errorName() + ": " + op->errorMessage());
        d->tpPendingChannel = NULL;
        d->errorString = QString("Telepathy Operation Failed: %1 - %2").arg(op->errorName(), op->errorMessage());
        emit this->error(d->errorString);
        return;
    }

    this->createHandler(d->tpPendingChannel->channel(), QDateTime());
    d->tpPendingChannel = NULL;

    emit this->voiceCallsChanged();
}

void TelepathyProvider::onHandlerInvalidated(const QString &errorName, const QString &errorMessage)
{
    TRACE
    Q_UNUSED(errorName)
    Q_UNUSED(errorMessage)
    Q_D(TelepathyProvider);

    TelepathyHandler *handler = qobject_cast<TelepathyHandler*>(QObject::sender());
    d->voiceCalls.remove(handler->handlerId());

    emit this->voiceCallRemoved(handler->handlerId());
    emit this->voiceCallsChanged();

    handler->deleteLater();
}
