#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , neighbors(new QList<Peer>)
    , routingTable(new QHash< QString,QPair<Peer,bool> >)
    , status_clock(new QTimer(this))
    , route_clock(new QTimer(this))
    , localSeqNo(1)
    , invalid(new Peer("0.0.0.0:0"))
{
    // parts incoming/outgoing message pipeline
    connect(this, SIGNAL(postToInbox(Message,Peer)), 
        this, SLOT(gotPostToInbox(Message,Peer)));
    connect(this, SIGNAL(monger(Message)),
        this, SLOT(gotMonger(Message)));
    connect(this, SIGNAL(needHelpFromPeer(Peer)),
        this, SLOT(gotNeedHelpFromPeer(Peer)));

    // periodic mongering
    connect(status_clock, SIGNAL(timeout()), 
        this, SLOT(status_chime()));
    connect(route_clock, SIGNAL(timeout()),
        this, SLOT(route_chime()));

    connect(this, SIGNAL(broadcast(Message)),
        this, SLOT(gotBroadcast(Message)));
    connect(this, SIGNAL(broadcastRoute()),
        this, SLOT(gotBroadcastRoute()));

    status_clock->start(STATUS_CLOCK_RATE);
    route_clock->start(ROUTE_CLOCK_RATE);
}

Mailbox::~Mailbox()
{}

void Mailbox::setMessageStore(MessageStore* m)
{
    msgstore = m;
}

void Mailbox::setDChatStore(DChatStore* d)
{
    dchatstore = d;
}

void Mailbox::setPortInfo(quint32 min, quint32 max, quint32 p)
{
    myPortMin = min;
    myPortMax = max;
    port = p;
    self = new Peer("127.0.0.1:"+QString::number(port));
}

void Mailbox::setID(QString str)
{
    ID = str;
}

Message Mailbox::routeRumor()
{
    Message route = Message();
    route.setType(TYPE_RUMOR_ROUTE);
    route.setOriginID(ID);
    route.setSeqNo(localSeqNo);
    // localSeqNo++;

    return route;
}

void Mailbox::populateNeighbors()
{
    QStringList clargs = QCoreApplication::arguments();
    for(int i = 1; i < clargs.size(); i++)
    {
        Peer peer = Peer(clargs.at(i));
        gotPotentialNewNeighbor(peer);
    }

    /**/
    if(port != myPortMin)
    {
        gotPotentialNewNeighbor(Peer("127.0.0.1:" + QString::number(port-1)));
    }
    if(port != myPortMax)
    {
        gotPotentialNewNeighbor(Peer("127.0.0.1:" + QString::number(port+1)));
    }
    /**/

    Q_EMIT(broadcastRoute());
}

Peer Mailbox::pickRandomPeer()
{
    return neighbors->at(qrand() % neighbors->size());
}

void Mailbox::gotPostToInbox(Message msg, Peer peer)
{
    QString msgOrigin = msg.getOriginID();

    if(msg.getType() == TYPE_RUMOR_CHAT || msg.getType() == TYPE_RUMOR_ROUTE)
    {
        processRumorRouteInfo(msg, peer);

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
        
        if(msgstore->isNewRumor(msg))
        {
            if(msgstore->isNextRumorInSeq(msg))
            {
                Q_EMIT(monger(msg)); // <-- this causes a lot of network overhead.
                if(msg.getType() == TYPE_RUMOR_CHAT)
                {
                    msgstore->addNewChatRumor(msg);
                    Q_EMIT(displayMessage(msg));
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
    else if(msg.getType() == TYPE_DIRECT_CHAT)
    {
        // double check to make sure this logic is complete + robust
        if(msg.getDest() == ID)
        {
            dchatstore->newDChat(msg);
        }
        else if(msg.getHopLimit() <= 0)
        {
            // drop
        }
        else if(!msgstore->isNewOrigin(msgOrigin)) // relay via routing table lookup
        {
            Peer forwardPeer = routingTable->value(msgOrigin).first;
            msg.setHopLimit(msg.getHopLimit() - 1); // decrement HopLimit
            Q_EMIT(sendMessage(msg, forwardPeer));
        }
    }
    else if(msg.getType() == TYPE_STATUS)
    {
        msgstore->processIncomingStatus(msg, peer);
    }

    processCommand(msg.getText());
}

void Mailbox::gotPostToOutbox(Message msg)
{
    msg.setOriginID(ID);

    if(msg.getType() == TYPE_RUMOR_CHAT)
    {
        msg.setSeqNo(localSeqNo);
        localSeqNo++;
        Peer peer = Peer("127.0.0.1:" + QString::number(port));
        Q_EMIT(postToInbox(msg, peer));
    }
    else if(msg.getType() == TYPE_DIRECT_CHAT)
    {
        Peer nextPeer = nextHop(msg.getDest());
        Q_EMIT(sendMessage(msg, nextPeer));

        dchatstore->newDChat(msg);
    }
}

void Mailbox::gotCanHelpPeer(Peer peer, QList<Message> list)
{
    QList<Message>::iterator i;
    for(i = list.begin(); i != list.end(); ++i)
    {
        Q_EMIT(sendMessage(*i, peer));
    }

    qDebug() << "HELPING PEER " << peer.toString() << " WITH " 
             << list.size() << " MESSAGES";
}

void Mailbox::gotNeedHelpFromPeer(Peer peer)
{
    Message status = msgstore->getStatus();
    Q_EMIT(sendMessage(status, peer));

    qDebug() << "NEED HELP FROM PEER " << peer.toString();
}

void Mailbox::gotInConsensusWithPeer()
{
    if(qrand() % 2 == 0)
    {
        Q_EMIT(monger(msgstore->getStatus()));
    }

    qDebug() << "IN CONSENSUS WITH PEER ";
}

void Mailbox::gotMonger(Message msg)
{
    if(!neighbors->isEmpty())
    {
        Peer peer = pickRandomPeer();
        Q_EMIT(sendMessage(msg, peer));
    }
}

void Mailbox::gotBroadcast(Message msg)
{
    if(neighbors != NULL && !neighbors->isEmpty())
    {
        foreach(Peer peer, *neighbors)
        {
            sendMessage(msg, peer);
        }
    }
}

void Mailbox::gotPotentialNewNeighbor(Peer peer)
{
    if(peer.isWellFormed() && !neighbors->contains(peer) && 
       peer != *invalid && peer != *self)
    {
        neighbors->append(peer);
        Q_EMIT(updateGUINeighbors(*neighbors));
        gotBroadcastRoute();
    }
}

void Mailbox::gotBroadcastRoute()
{
    Message route = routeRumor();
    Q_EMIT(broadcast(route));
}

void Mailbox::status_chime()
{
    Message status = msgstore->getStatus();
    Q_EMIT(monger(status));
}

void Mailbox::route_chime()
{
    Q_EMIT(broadcastRoute());
    qDebug() << "PERIODIC ROUTE BROADCAST: " << routeRumor().toString();
}

void Mailbox::gotSendStatusToPeer(Peer peer)
{
    Message status = msgstore->getStatus();
    Q_EMIT(sendMessage(status, peer));
}

void Mailbox::processCommand(QString cmd)
{
    if(cmd == CMD_PRINT_MSGSTORE)
    {
        qDebug() << "\nMESSAGE STORE:";
        qDebug() << msgstore->toString();
    }
    else if(cmd == CMD_PRINT_STATUS)
    {
        qDebug() << "\nOWN STATUS:";
        qDebug() << msgstore->getStatus().toString();
    }
    else if(cmd == CMD_PRINT_NEIGHBORS)
    {
        qDebug() << "\nNEIGHBORS:";
        foreach(Peer peer, *neighbors)
        {
            qDebug() << "   " << peer.toString();
        }
    }
    else if(cmd == CMD_PRINT_TABLE)
    {
        qDebug() << "\nROUTING TABLE:";
        foreach(QString qstr, routingTable->keys())
        {
            qDebug() << "   <ORIGIN: " << qstr <<  
                            ", PEER: " << nextHop(qstr).toString() << 
                            ", DIRECT: " << nextHopIsDirect(qstr) << ">";
        }
    }
}

void Mailbox::processRumorRouteInfo(Message msg, Peer peer)
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
        routingTable->insert(msgOrigin, tableEntry);
        if(msgstore->isNewOrigin(msgOrigin))
        {
            Q_EMIT(updateGUIOriginsList(msgOrigin));
            Q_EMIT(broadcastRoute());
        }
    }
    else if(msgstore->isNewOrigin(msgOrigin))
    {
        tableEntry.second = false;
        routingTable->insert(msgOrigin, tableEntry);
        Q_EMIT(updateGUIOriginsList(msgOrigin));
        Q_EMIT(broadcastRoute());

    }
    else if(msgstore->isNewRumor(msg) && !nextHopIsDirect(msgOrigin))
    {
        tableEntry.second = false;
        routingTable->insert(msgOrigin, tableEntry);
    }
}

Peer Mailbox::nextHop(QString origin)
{
    if(routingTable->keys().contains(origin))
    {
        return routingTable->value(origin).first;
    }
    else
    {
        return *invalid;
    }
}

bool Mailbox::nextHopIsDirect(QString origin)
{
    if(msgstore->isNewOrigin(origin))
    {
        return false;
    }
    return routingTable->value(origin).second == true;
}



