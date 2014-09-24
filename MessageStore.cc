#include "MessageStore.hh"

MessageStore::MessageStore(Peerster* p)
    : peerster(p)
    , store(new QMap< QString, QList<Message> >())
{}

MessageStore::~MessageStore()
{}

bool MessageStore::isNewRumor(Message msg)
{
    if(!(msg.getType() == TYPE_RUMOR_CHAT) && !(msg.getType() == TYPE_RUMOR_ROUTE))
    {
        return false;
    }
    
    if(msg.getType() == TYPE_RUMOR_CHAT && !store->contains(msg.getOriginID()))
    {
        return true;
    }

    quint32 msgSeqNo = msg.getSeqNo();
    QMap<QString, quint32> latest = getLatest();
    
    if(msgSeqNo > latest.value(msg.getOriginID()))
    {
        return true;
    }
    
    return false;
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

    store->insert(origin, originHistory);
}


QList<Message> MessageStore::getMessagesInRange(QString origin, 
    quint32 firstSeqNo, quint32 lastSeqNo)
{   // range is inclusive on both ends.
    Message curr;
    quint32 currSeqNo;
    QList<Message> originHistory, ret;

    originHistory = store->value(origin);
    ret = QList<Message>();

    if(firstSeqNo > getLatest().value(origin))
        return ret;

    QList<Message>::iterator i;
    for(i = originHistory.begin(); i != originHistory.end(); ++i)
    {
        // TODO: inefficient access, consider assuming well-orderedness
        // and contiguity of SeqNo's in history and access element
        // index by seqNo - 1
        curr = *i;
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

    QMap<QString, quint32> latest = getLatest();

    QMap<QString, QList<Message> >::iterator i;
    for(i = store->begin(); i != store->end(); ++i)
    {
        want.insert(i.key(), latest.value(i.key())+1);
    }
    status.setWantMap(want);
    status.setType(TYPE_STATUS);

    return status;
}

QMap<QString, quint32> MessageStore::getLatest()
{
    QMap<QString, quint32> latest;

    QMap<QString, QList<Message> >::iterator i;
    for(i = store->begin(); i != store->end(); ++i)
    {
        latest.insert(i.key(), i.value().back().getSeqNo());
    }

    return latest;
}

bool MessageStore::isNextInSeq(Message msg)
{
    QMap<QString, quint32> latest = getLatest();
    if(store->contains(msg.getOriginID()))
    {
        if(msg.getSeqNo() == latest.value(msg.getOriginID())+1)
            return true;
        else
            return false;
    }
    else if(msg.getSeqNo() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}   

void MessageStore::processIncomingStatus(Message status, Peer peer)
{
    QVariantMap ownWant = getStatus().getWantMap();
    QVariantMap incomingWant = status.getWantMap();
    quint32 ownWantSeqNo, incomingWantSeqNo;
    Message curr;
    QList<QString> inOwnButNotIncoming;
    QList<Message> toSend;
    bool needHelp = false;

    QVariantMap::iterator i;
    QList<Message>::iterator j;
    for(i = incomingWant.begin(); i != incomingWant.end(); ++i)
    {
        if(ownWant.contains(i.key()))
        {   // compare notes on seqNo
            ownWantSeqNo = ownWant.value(i.key()).toInt();
            incomingWantSeqNo = incomingWant.value(i.key()).toInt();
            
            if(ownWantSeqNo < incomingWantSeqNo)
            {
                needHelp = true;
            }
            else if(ownWantSeqNo > incomingWantSeqNo)
            {   // prepare list
                toSend = getMessagesInRange(i.key(), 
                    incomingWantSeqNo-1, ownWantSeqNo-1);
                Q_EMIT(canHelpPeer(peer, toSend));
            }
        }   
        else
        {
            needHelp = true;
        }
    }

    // handle msgs that peer has, but we don't.
    for(i = ownWant.begin(); i != ownWant.end(); ++i)
    {
        if(!incomingWant.contains(i.key()))
            inOwnButNotIncoming.append(i.key());
    }

    if(!inOwnButNotIncoming.isEmpty())
    {
        QList<QString>::iterator k;
        for(k = inOwnButNotIncoming.begin(); k != inOwnButNotIncoming.end(); ++k)
        {
            toSend = getMessagesInRange(*k, 1, getLatest().value(*k));
            Q_EMIT(canHelpPeer(peer, toSend));
        }
    }

    if(needHelp)
    {
        Q_EMIT(needHelpFromPeer(peer));
    }

    if(!needHelp && inOwnButNotIncoming.isEmpty())
    {
        Q_EMIT(inConsensusWithPeer());
    }
}

QString MessageStore::toString()
{
    QString qstr;
    QList<Message> senderHistory;
    QMap<QString, QList<Message> >::iterator i;
    QList<Message>::iterator j;

    for(i = store->begin(); i != store->end(); ++i)
    {
        qstr += "<ID: " + i.key() + ">, <";
        senderHistory = i.value();
        for(j = senderHistory.begin(); j != senderHistory.end(); ++j)
        {
            qstr += QString::number(j->getSeqNo()) + ", ";
        }
        qstr += ">\n";
    }

    return qstr;
}
