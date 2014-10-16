#include "MessageStore.hh"

MessageStore::MessageStore(Peerster* p)
    : peerster(p)
    , rumorStore(new QMap< QString,QList<Message> >())
    , table(new QHash< QString,QPair<Peer,bool> >())
    , invalid(new Peer("0.0.0.0:0"))
    , localSeqNo(1)
{
    connect(this, SIGNAL(processRumor(Message,Peer)),
        this, SLOT(gotProcessRumor(Message,Peer)));
    connect(this, SIGNAL(processRumorRoute(Message,Peer)),
        this, SLOT(gotProcessRumorRoute(Message,Peer)));
    connect(this, SIGNAL(processDirectChat(Message)),
        this, SLOT(gotProcessDirectChat(Message)));
    connect(this, SIGNAL(sendDirect(Message,QString)),
        this, SLOT(gotSendDirect(Message,QString)));
}

MessageStore::~MessageStore()
{}

void MessageStore::setID(QString id)
{
    ID = id;
}

void MessageStore::setGroupConvo(QList<Message>* gc)
{
    groupConvo = gc;
}

void MessageStore::setDirectStore(QMap< QString,QList<Message> >* ds)
{
    directStore = ds;
}

void MessageStore::setSelfPeer(Peer* s)
{
    self = s;
}

QString MessageStore::toString()
{
    QString qstr;
    QList<Message> senderHistory;
    
    QMap<QString, QList<Message> >::iterator i;
    for(i = rumorStore->begin(); i != rumorStore->end(); ++i)
    {
        qstr += "<ID: " + i.key() + ">, <";
        senderHistory = i.value();
        foreach(Message msg, senderHistory)
        {
            qstr += QString::number(msg.getSeqNo()) + ", ";
        }
        qstr += ">\n";
    }

    return qstr;
}

void MessageStore::gotCreateChatRumor(QString text)
{
    Message msg = Message();

    msg.setType(TYPE_RUMOR_CHAT);
    msg.setChatText(text);
    msg.setOriginID(ID);
    msg.setSeqNo(localSeqNo);
    localSeqNo++;

    Q_EMIT(processRumor(msg, *self));
}

void MessageStore::gotCreateDirectChat(QString text, QString dest)
{
    Message msg = Message();

    msg.setType(TYPE_DIRECT_CHAT);
    msg.setDest(dest);
    msg.setHopLimit(DCHAT_HOP_LIMIT);
    msg.setChatText(text);

    Q_EMIT(processDirectChat(msg));
}

void MessageStore::gotProcessRumor(Message msg, Peer peer)
{
    qDebug() << "LOCAL SEQNO: " << localSeqNo << "  ID: " << ID;
    qDebug() << "PROCESS: " << msg.toString();

    // is this conditional block really necessary?
    if(isNewOrigin(msg.getOriginID()))
    {
        rumorStore->insert(msg.getOriginID(), QList<Message>());
        Q_EMIT(refreshOrigins(rumorStore->keys()));
    }

    if(msg.getOriginID() != ID && peer != *invalid && peer != *self)
    {   // handle potential new neighbors
        if(!msg.isDirectRumor())
        {
            Peer neighbor = Peer(msg.getLastIP(), msg.getLastPort());
            Q_EMIT(processNeighbor(neighbor));
        }

        msg.setLastIP(peer.getAddress().toIPv4Address());
        msg.setLastPort(peer.getPort());
    }

    if(isNewRumor(msg))
    {
        qDebug() << msg.toString() << " IS NEW RUMOR!";
        if(isNextRumorInSeq(msg))
        {
            qDebug() << msg.toString() << " IS NEXT RUMOR IN SEQ!";
            Q_EMIT(monger(msg)); // <-- this could cause a lot of network overhead.
            addRumor(msg);
            if(msg.getType() == TYPE_RUMOR_CHAT)
            {
                qDebug() << msg.toString() << " IS CHATRUMOR!";
                groupConvo->append(msg);
                Q_EMIT(refreshGroupConvo());
            }
        }
        else if(msg.getOriginID() != ID)
        {
            qDebug() << msg.toString() << " IS NOT NEXT RUMOR IN SEQ!";
            qDebug() << "STATUS: " << status().toString();
            Q_EMIT(needHelpFromPeer(peer));
        }
    }
    else
    {
        qDebug() << "GOT OLD RUMOR:" << msg.toString();
        // do nothing
    }
}

void MessageStore::gotProcessRumorRoute(Message msg, Peer peer)
{
    QString msgOrigin = msg.getOriginID();
    QPair<Peer,bool> tableEntry;

    tableEntry.first = peer;

    if(msgOrigin == ID)
    {
        return;
    }
    else if(msg.isDirectRumor())
    {
        tableEntry.second = true;
        table->insert(msgOrigin, tableEntry);
    }
    else if(isNewOrigin(msgOrigin) ||
            (isNewRumor(msg) && !nextHopIsDirect(msgOrigin)))
    {
        tableEntry.second = false;
        table->insert(msgOrigin, tableEntry);
    }
}

void MessageStore::gotProcessDirectChat(Message msg)
{
    // double check to make sure this logic is complete + robust
    if(msg.getDest() == ID)
    {
        addDirectChat(msg);
    }
    else if(msg.getOriginID() == ID)
    {
        addDirectChat(msg);
        Q_EMIT(sendDirect(msg, msg.getDest()));
    }
    // else if(isNewOrigin(msg.getOriginID()))
    // {
    //     // TODO: what to do with new origin?
    //     // probably nothing, because
    //     // we only construct the routing
    //     // table using rumors, not DirectChat's

    //     // if we do build table from DC's,
    //     // we can use this to improve table.
    // }
    else if(msg.getHopLimit() > 0)
    {
        msg.setHopLimit(msg.getHopLimit() - 1); // decrement HopLimit
        Q_EMIT(sendDirect(msg, msg.getDest())); // send it on its way
    }
}

void MessageStore::gotProcessIncomingStatus(Message msg, Peer peer)
{
    QVariantMap ownWant = status().getWantMap();
    QVariantMap incomingWant = msg.getWantMap();
    quint32 ownWantSeqNo, incomingWantSeqNo;
    Message curr;
    QList<QString> inOwnButNotIncoming;
    QList<Message> toSend;
    bool needHelp = false;

    qDebug() << "OWN STATUS: " << status().toString();
    qDebug() << peer.toString() << "'s STATUS" << msg.toString();

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
            toSend = getMessagesInRange(*k, 1, latest().value(*k));
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

void MessageStore::gotSendDirect(Message msg,QString msgOrigin)
{
    Peer forwardPeer = table->value(msgOrigin).first;
    Q_EMIT(sendMessage(msg, forwardPeer));
}

void MessageStore::gotBroadcastRoute()
{
    Message route = routeRumor();
    Q_EMIT(broadcast(route));
}

bool MessageStore::isNewRumor(Message msg)
{
    if(!(msg.getType() == TYPE_RUMOR_CHAT || msg.getType() == TYPE_RUMOR_ROUTE))
    {
        return false;
    }
    else if(isNewOrigin(msg.getOriginID()))
    {
        return true;
    }

    quint32 msgSeqNo = msg.getSeqNo();
    
    if(msgSeqNo > latest().value(msg.getOriginID()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool MessageStore::isNewOrigin(QString origin)
{
    return !(rumorStore->keys().contains(origin));
}

bool MessageStore::isNextRumorInSeq(Message msg)
{
    if(rumorStore->contains(msg.getOriginID()))
    {
        if(msg.getSeqNo() == latest().value(msg.getOriginID())+1)
        {
            return true;
        }
        else
        {
            return false;
        }
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

bool MessageStore::nextHopIsDirect(QString origin)
{
    if(isNewOrigin(origin))
    {
        return false;
    }
    else
    {
        return (table->value(origin).second) == true;
    }
}

Peer MessageStore::nextHop(QString origin)
{
    if(isNewOrigin(origin))
    {
        return *invalid;
    }
    else
    {
        return table->value(origin).first;
    }
}

void MessageStore::addRumor(Message msg)
{
    qDebug() << "ADD " << msg.toString() << "TO RUMORSTORE";

    QString origin = msg.getOriginID();

    if(isNewOrigin(origin)) // add new origin
    {
        rumorStore->insert(origin, QList<Message>());
        Q_EMIT(refreshOrigins(rumorStore->keys()));
        Q_EMIT(broadcast(routeRumor()));
    }

    QList<Message> originRumors = rumorStore->value(origin);
    originRumors.append(msg);

    rumorStore->insert(origin, originRumors);

    qDebug() << "HISTORY OF ORIGIN " << msg.getOriginID() << ": "; 
    foreach(Message m, rumorStore->value(origin))
    {
        qDebug() << m.toString();
    }


    Q_EMIT(updateStatus(status()));
}

void MessageStore::addDirectChat(Message msg)
{
    QString origin = (msg.getOriginID() != ID) ? msg.getOriginID() 
                                                    : msg.getDest(); 
    QList<Message> directConvo;
    if(directStore->contains(origin))
    {
        directConvo = directStore->value(origin);
    }
    else
    {
        directConvo = QList<Message>();
    }
    directConvo.append(msg);

    directStore->insert(origin, directConvo); 

    qDebug() << "DIRECT CONVO FOR " << origin << " LENGTH = " << directConvo.size();

    Q_EMIT(refreshDirectConvo(origin));
}

QList<Message> MessageStore::getMessagesInRange(QString origin, 
    quint32 firstSeqNo, quint32 lastSeqNo)
{   // range is inclusive on both ends.
    Message curr;
    quint32 currSeqNo;
    QList<Message> originHistory, ret;

    originHistory = rumorStore->value(origin);
    ret = QList<Message>();

    if(firstSeqNo > latest().value(origin))
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

QMap<QString, quint32> MessageStore::latest()
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

Message MessageStore::status()
{
    Message status;
    status.setType(TYPE_STATUS);

    QVariantMap want;
    QMap<QString, quint32> mostRecent = latest();

    foreach(QString origin, rumorStore->keys())
    {
        want.insert(origin, mostRecent.value(origin)+1);
    }
    status.setWantMap(want);

    return status;
}

Message MessageStore::routeRumor()
{
    Message route = Message();

    route.setType(TYPE_RUMOR_ROUTE);
    route.setOriginID(ID);
    route.setSeqNo(localSeqNo);
    localSeqNo++;
    Q_EMIT(processRumor(route, *self));

    return route;
}


