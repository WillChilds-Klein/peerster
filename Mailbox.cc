#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , neighbors(new QList<Peer>())
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

void Mailbox::setRoutingTable(RoutingTable* r)
{
    table = r;
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

    if(port != myPortMin)
    {
        gotPotentialNewNeighbor(Peer("127.0.0.1:" + QString::number(port-1)));
    }
    if(port != myPortMax)
    {
        gotPotentialNewNeighbor(Peer("127.0.0.1:" + QString::number(port+1)));
    }

    gotBroadcastRoute();

    qDebug() << "NEIGHBORS:";
    foreach(Peer peer, *neighbors)
    {
        qDebug() << "   " << peer.toString();
    }
}

Peer Mailbox::pickRandomPeer()
{
    return neighbors->at(qrand() % neighbors->size());
}

void Mailbox::gotPostToInbox(Message msg, Peer peer)
{
    if(msg.getType() == TYPE_RUMOR_CHAT || msg.getType() == TYPE_RUMOR_ROUTE)
    {

        if(msg.getOriginID() != ID && peer != *self && peer != *invalid)
        {
            if(!msg.isDirectRumor())
            {
                Q_EMIT(gotPotentialNewNeighbor(Peer(msg.getLastIP(), msg.getLastPort())));
            }

            msg.setLastIP(peer.getAddress().toIPv4Address());
            msg.setLastPort(peer.getPort());
        }

        if(msg.getOriginID() != ID)
        {
            Q_EMIT(updateTable(msg, peer));
        }
        
        if(msgstore->isNewRumor(msg) && msg.getType() == TYPE_RUMOR_CHAT)
        {
            if(msgstore->isNextInSeq(msg))
            {
                Q_EMIT(monger(msg));
                msgstore->addNewRumor(msg);
                Q_EMIT(displayMessage(msg));
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
            if(table->isNewOrigin(msg.getOriginID()))
            {
                Q_EMIT(updateTable(msg, peer));
            }
            dchatstore->newDChat(msg);
        }
        else if(msg.getHopLimit() <= 0)
        {
            // drop
        }
        else // relay via routing table lookup
        {
            Peer forwardPeer = table->get(msg.getDest());
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
        Peer nextHop = table->get(msg.getDest());
        Q_EMIT(sendMessage(msg, nextHop));

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
    // qDebug() << "Peer" << peer.toString() << "needs help with " 
    //                                       << list.size() << "messages!";
}

void Mailbox::gotNeedHelpFromPeer(Peer peer)
{
    // qDebug() << "Need Help from Peer" << peer.toString();
    Message status = msgstore->getStatus();
    Q_EMIT(sendMessage(status, peer));
}

void Mailbox::gotInConsensusWithPeer()
{
    if(qrand() % 2 == 0)
    {
        Q_EMIT(monger(msgstore->getStatus()));
    }
}

void Mailbox::gotMonger(Message msg)
{
    if(!neighbors->isEmpty())
    {
        Peer peer = pickRandomPeer();
        Q_EMIT(sendMessage(msg, peer));
    }
}

void Mailbox::gotPotentialNewNeighbor(Peer peer)
{
    if(peer.isWellFormed() && !neighbors->contains(peer) && 
       peer != *invalid && peer != *self)
    {
        neighbors->append(peer);
        // Message route = routeRumor();
        // Q_EMIT(sendMessage(route, peer));
        Q_EMIT(updateGUINeighbors(*neighbors));
        gotBroadcastRoute();
        // qDebug() << "NEW NEIGHBOR:" << peer.toString();
    }
}

void Mailbox::gotBroadcastRoute()
{
    Message route = routeRumor();
    foreach(Peer peer, *neighbors)
    {
        sendMessage(route, peer);
    }
}

void Mailbox::status_chime()
{
    Message status = msgstore->getStatus();
    Q_EMIT(monger(status));
}

void Mailbox::route_chime()
{
    // Message route = routeRumor();
    // Q_EMIT(monger(route));
    gotBroadcastRoute();
    qDebug() << "BROADCAST ROUTE: " << routeRumor().toString();
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
            qDebug() << "   " << peer.toString();
    }
    else if(cmd == CMD_PRINT_TABLE)
    {
        qDebug() << "\nROUTING TABLE:";
        foreach(QString qstr, table->origins())
            qDebug() << "   <ORIGIN: " << qstr <<  
                            ", PEER: " << table->get(qstr).toString() << ">";
    }
}


