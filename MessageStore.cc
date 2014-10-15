#include "MessageStore.hh"

MessageStore::MessageStore(Peerster* p)
    : peerster(p)
    , localSeqNo(1)
    , rumorStore(new QMap< QString, QList<Message> >())
{}

MessageStore::~MessageStore()
{}

void MessageStore::setGroupConvo(QList<Message>* gc)
{
    groupConvo = gc;
}

void MessageStore::setDirectStore(QMap< QString,QList<Message> >* ds)
{
    directStore = ds;
}

bool MessageStore::isNewRumor(Message msg)
{
    if(!(msg.getType() == TYPE_RUMOR_CHAT || msg.getType() == TYPE_RUMOR_ROUTE))
    {
        return false;
    }
    
    if(isNewOrigin(msg.getOriginID()))
    {
        addNewOrigin(msg.getOriginID());
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

bool MessageStore::isNewOrigin(QString origin)
{
    return !rumorStore->keys().contains(origin);
}

bool MessageStore::isNextRumorInSeq(Message msg)
{
    QMap<QString, quint32> latest = getLatest();
    if(rumorStore->contains(msg.getOriginID()))
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

void MessageStore::addNewOrigin(QString origin)
{
    if(isNewOrigin(origin))
    {
        rumorStore->insert(origin, QList<Message>());
    }
}

void MessageStore::addNewChatRumor(Message msg)
{
    QString origin = msg.getOriginID();
    QList<Message> originHistory;
    
    originHistory = rumorStore->value(origin);
    originHistory.append(msg);

    rumorStore->insert(origin, originHistory);
}


QList<Message> MessageStore::getMessagesInRange(QString origin, 
    quint32 firstSeqNo, quint32 lastSeqNo)
{   // range is inclusive on both ends.
    Message curr;
    quint32 currSeqNo;
    QList<Message> originHistory, ret;

    originHistory = rumorStore->value(origin);
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
    for(i = rumorStore->begin(); i != rumorStore->end(); ++i)
    {
        want.insert(i.key(), latest.value(i.key())+1);
    }
    status.setWantMap(want);
    status.setType(TYPE_STATUS);

    return status;
}

QString MessageStore::toString()
{
    QString qstr;
    QList<Message> senderHistory;
    QMap<QString, QList<Message> >::iterator i;
    QList<Message>::iterator j;

    for(i = rumorStore->begin(); i != rumorStore->end(); ++i)
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

QMap<QString, quint32> MessageStore::getLatest()
{
    QMap<QString, quint32> latest;

    QMap<QString, QList<Message> >::iterator i;
    for(i = rumorStore->begin(); i != rumorStore->end(); ++i)
    {
        if(i.value().isEmpty())
        {
            latest.insert(i.key(), 0);
        }
        else
        {
            latest.insert(i.key(), i.value().back().getSeqNo());
        }
    }

    return latest;
}

void MessageStore::newDChat(Message dmsg)
{
    QString origin = (dmsg.getOriginID() != ID) ? dmsg.getOriginID() 
                                                : dmsg.getDest(); 
    QList<Message> directConvo;
    if(directStore->contains(origin))
    {
        directConvo = directStore->value(origin);
    }
    else
    {
        directConvo = QList<Message>();
    }

    directConvo.append(dmsg);  
    directStore->insert(origin, directConvo); 

    qDebug() << "DIRECT CONVO FOR " << origin << " LENGTH = " << directConvo.size();

    Q_EMIT(updateGUIDChatHistory(origin, directConvo));
}

void MessageStore::gotProcessRumor(Message msg)
{
    Q_EMIT(processRumorRoute(msg, peer));

    if(msgOrigin != ID && peer != *self && peer != *invalid)
    {
        // handle potential new neighbors
        if(!msg.isDirectRumor())
        {
            Peer neighbor = Peer(msg.getLastIP(), msg.getLastPort());
            Q_EMIT(gotPotentialNewNeighbor(neighbor));
        }

        msg.setLastIP(peer.getAddress().toIPv4Address());
        msg.setLastPort(peer.getPort());
    }
    
    if(isNewRumor(msg))
    {
        if(isNextRumorInSeq(msg))
        {
            Q_EMIT(monger(msg)); // <-- this causes a lot of network overhead.
            if(msg.getType() == TYPE_RUMOR_CHAT)
            {
                addNewChatRumor(msg);
                Q_EMIT(refreshGroupConvo());
            }
        }
        else
        {
            Q_EMIT(needHelpFromPeer(peer));
        }
    }
    else
    {
        qDebug() << "GOT OLD RUMOR:" << msg.toString();
        // do nothing
    }
}

void MessageStore::gotProcessDirectChat(Message msg)
{
    // double check to make sure this logic is complete + robust
    if(msg.getDest() == ID)
    {
        newDChat(msg);
    }
    else if(msg.getOriginID() == ID)
    {
        newDChat(msg);
        sendDirect(msg, msg.getDest());
    }
    else if(isNewOrigin(msgOrigin))
    {
        // what to do with new origin?
    }
    else if(msg.getHopLimit() > 0)
    {
        msg.setHopLimit(msg.getHopLimit() - 1); // decrement HopLimit
        Q_EMIT(sendDirect(msg));                // send it on its way
    }
}

void MessageStore::gotProcessIncomingStatus(Message msg, Peer peer)
{
    QVariantMap ownWant = getStatus().getWantMap();
    QVariantMap incomingWant = status.getWantMap();
    quint32 ownWantSeqNo, incomingWantSeqNo;
    Message curr;
    QList<QString> inOwnButNotIncoming;
    QList<Message> toSend;
    bool needHelp = false;

    qDebug() << "OWN STATUS: " << getStatus().toString();
    qDebug() << peer.toString() << "'s STATUS" << status.toString();

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
            else if(ownWantSeqNo > incomingWantSeqNo && 
                    !rumorStore->value(i.key()).isEmpty())
            {   // prepare list
                toSend = getMessagesInRange(i.key(), 
                    incomingWantSeqNo-1, ownWantSeqNo-1);
                Q_EMIT(helpPeer(peer, toSend));
            }
            else
            {
                // in consensus for current origin
            }
        }   
        else
        {
            needHelp = true;
            addNewOrigin(i.key());
        }
    }

    // handle msgs that peer has, but we don't.
    for(i = ownWant.begin(); i != ownWant.end(); ++i)
    {
        if(!incomingWant.contains(i.key()))
        {
            inOwnButNotIncoming.append(i.key());
        }
    }

    if(!inOwnButNotIncoming.isEmpty())
    {
        QList<QString>::iterator k;
        for(k = inOwnButNotIncoming.begin(); k != inOwnButNotIncoming.end(); ++k)
        {
            toSend = getMessagesInRange(*k, 1, getLatest().value(*k));
            Q_EMIT(helpPeer(peer, toSend));
        }
    }

    if(needHelp)
    {
        Q_EMIT(needHelpFromPeer(peer));
    }
    else if(inOwnButNotIncoming.isEmpty())
    {
        Q_EMIT(inConsensusWithPeer());
    }
}

void MessageStore::gotGetDChatHistoryFromOrigin(QString origin)
{
    QList<Message> directConvo;
    if(directStore->contains(origin))
    {
        directConvo = directStore->value(origin);
    }
    else
    {
        directConvo = QList<Message>();
    }

    Q_EMIT(updateGUIDChatHistory(origin, directConvo));
}

void MessageStore::setID(QString qstr)
{
    ID = qstr;
}

Message MessageStore::routeRumor()
{
    Message route = Message();
    route.setType(TYPE_RUMOR_ROUTE);
    route.setOriginID(ID);
    route.setSeqNo(localSeqNo);
    // localSeqNo++;

    return route;
}


