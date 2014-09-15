#include "MessageStore.hh"

MessageStore::MessageStore(Peerster* p)
    : peerster(p)
    , store(new QMap< QString, QList<Message> >())
    , latest(new QMap<QString, quint32>())
{}

MessageStore::~MessageStore()
{}

bool MessageStore::isNewRumor(Message msg)
{
    if(!msg.typeIsRumor())
        return false;
    else if(!store->contains(msg.getOriginID()))
        return true;

    quint32 msgSeqNo = msg.getSeqNo();

    QList<Message> senderHistory = store->value(msg.getOriginID());

    if(!senderHistory.isEmpty() && 
        senderHistory.back().getSeqNo() > msgSeqNo)
        return false;
    else
        return true;
}

bool MessageStore::isNewOrigin(Message msg)
{
    return store->contains(msg.getOriginID());
}

void MessageStore::addNewRumor(Message msg)
{
    QString origin = msg.getOriginID();
    QList<Message> originHistory;
    if(store->contains(origin))
    {
        originHistory = store->value(origin);
    }
    else
    {
        originHistory = QList<Message>();
    }

    originHistory.append(msg);
    latest->insert(origin, msg.getSeqNo());

    store->insert(origin, originHistory);
}


QList<Message> MessageStore::getMessagesInRange(QString origin, 
    quint32 firstSeqNo, quint32 lastSeqNo)
{
    Message curr;
    quint32 currSeqNo;
    QList<Message> originHistory, ret;

    originHistory = store->value(origin);
    ret = QList<Message>();

    if(firstSeqNo > latest->value(origin))
        return ret;

    QListIterator<Message> i(originHistory);
    while (i.hasNext())
    {
        // inefficient access, consider assuming well-orderedness
        // and contiguity of SeqNo's in history and access element
        // index by seqNo - 1
        curr = i.next();
        currSeqNo = curr.getSeqNo();
        if(currSeqNo >= firstSeqNo && currSeqNo <= lastSeqNo)
            ret.append(curr);
    }

    return ret;
}

Message MessageStore::getStatus()
{
    Message status;
    QVariantMap want;

    QMap<QString, quint32>::iterator i;
    for(i = latest->begin(); i != latest->end(); i++)
    {
        want.insert(i.key(), (i.value() + 1));
    }
    status.insert(WANT_KEY, want);

    return status;
}

bool MessageStore::isNextInSeq(Message msg)
{
    if(store->contains(msg.getOriginID())
    {
        if(msg.getSeqNo() == (latest->value(msg.getOriginID()) + 1))
            return true;
        else
            return false;
    }
    else
    {
        if(msg.getSeqNo() == 1)
            return true;
        else
            return false;
    }
}   

void MessageStore::processIncomingStatus(Message status)
{
    QVariantMap ownWant = getStatus().getWantMap();
    QVariantMap incomingWant = status.getWantMap();
    quint32 ownSeqNo, incomingSeqNo;
    Peer incoming = Peer(status.getPortOfOrigin());

    QList<QString> inOwnButNotIncoming;
    bool needHelp = false;

    QVariantMap::iterator i;
    for(i = incomingWant.begin(); i != incomingWant.end(); i++)
    {
        if(ownWant.contains(i.key()))
        {
            // compare notes on seqNo
            if()
        }   
        else
        {
            needHelp = true;
            status->insert(i.key(), 0);
        }
    }

    if(needHelp)
    {
        Q_EMIT(needHelpFromPeer(incoming));
    }

    for(i = ownWant.begin(); i != ownWant.end(); j++)
    {
        if(!incomingWant.contains(j.key()))
            inOwnButNotIncoming.append(j.key());
    }

    if(!inOwnButNotIncoming.isEmpty())
    {
        QListIterator<QString> j(inOwnButNotIncoming);
        QString curr;
        while (j.hasNext())
        {
            curr = j.next();
            // send rumors in order
        }
        Q_EMIT(canHelpPeer(incoming));
    }
}


