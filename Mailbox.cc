#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , neighbors(new QList<Peer>())
    , status_clock(new QTimer(this))
    , route_clock(new QTimer(this))
    , invalid(new Peer("0.0.0.0:0"))
    , status(new Message())
{
    // incoming/outgoing message pipeline
    connect(this, SIGNAL(monger(Message)),
        this, SLOT(gotMonger(Message)));
    connect(this, SIGNAL(broadcast(Message)),
        this, SLOT(gotBroadcast(Message)));

    // neighbor processing
    connect(this, SIGNAL(processNeighbor(Peer)),
        this, SLOT(gotProcessNeighbor(Peer)));

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

void Mailbox::setPortInfo(quint32 min, quint32 max, quint32 p)
{
    myPortMin = min;
    myPortMax = max;
    port = p;
}

void Mailbox::setSelfPeer(Peer* s)
{
    self = s;
}

void Mailbox::populateNeighbors()
{
    QStringList clargs = QCoreApplication::arguments();
    for(int i = 1; i < clargs.size(); i++)
    {
        Peer peer = Peer(clargs.at(i));
        Q_EMIT(processNeighbor(peer));
    }

    /**/
    if(port != myPortMin)
    {
        Q_EMIT(processNeighbor(Peer("127.0.0.1:" + QString::number(port-1))));
    }
    if(port != myPortMax)
    {
        Q_EMIT(processNeighbor(Peer("127.0.0.1:" + QString::number(port+1))));
    }
    /**/

    Q_EMIT(broadcastRoute());
}

void Mailbox::gotPostToInbox(Message msg, Peer peer)
{
    QString msgOrigin = msg.getOriginID();

    if(msg.getType() == TYPE_RUMOR_CHAT || msg.getType() == TYPE_RUMOR_ROUTE)
    {
        Q_EMIT(processRumorRoute(msg, peer));
        Q_EMIT(processRumor(msg, peer));
    }
    else if(msg.getType() == TYPE_DIRECT_CHAT)
    {
        Q_EMIT(processDirectChat(msg));
    }
    else if(msg.getType() == TYPE_STATUS)
    {
        Q_EMIT(processIncomingStatus(msg, peer));
    }
    else if(msg.getType() == TYPE_BLOCK_REQUEST)
    {
        Q_EMIT(processBlockRequest(msg));
    }
    else if(msg.getType() == TYPE_BLOCK_REPLY)
    {
        Q_EMIT(processBlockReply(msg));
    }
    else if(msg.getType() == TYPE_SEARCH_REQUEST)
    {
        if(msg.getOriginID() != ID)
        {
            Q_EMIT(processSearchRequest(msg));
        }
        forwardSearchRequest(msg);
    }
    else if(msg.getType() == TYPE_SEARCH_REPLY)
    {
        Q_EMIT(processSearchReply(msg));
    }

    processCommand(msg.getText());
}

void Mailbox::gotProcessNeighbor(Peer peer)
{
    if(peer.isWellFormed() && !neighbors->contains(peer) && 
       peer != *invalid && peer != *self)
    {
        neighbors->append(peer);
        Q_EMIT(refreshNeighbors(*neighbors));
        Q_EMIT(sendMessage(*status, peer));
        Q_EMIT(broadcastRoute());
    }
}

void Mailbox::gotHelpPeer(Peer peer, QList<Message> list)
{
    QList<Message>::iterator i;
    for(i = list.begin(); i != list.end(); ++i)
    {
        Q_EMIT(sendMessage(*i, peer));
    }

    // qDebug() << "HELPING PEER " << peer.toString() << " WITH
}

void Mailbox::gotNeedHelpFromPeer(Peer peer)
{
    Q_EMIT(sendMessage(*status, peer));

    // qDebug() << "NEED HELP FROM PEER " << peer.toString();
}

void Mailbox::gotInConsensusWithPeer()
{
    if(qrand() % 2 == 0)
    {
        Q_EMIT(monger(*status));
    }

    // qDebug() << "IN CONSENSUS WITH PEER ";
}

void Mailbox::gotUpdateStatus(Message msg)
{
    *status = msg;
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

void Mailbox::status_chime()
{
    Q_EMIT(monger(*status));
}

void Mailbox::route_chime()
{
    Q_EMIT(broadcastRoute());
    qDebug() << "PERIODIC ROUTE BROADCAST";
}

Peer Mailbox::pickRandomPeer()
{
    return neighbors->at(qrand() % neighbors->size());
}

void Mailbox::forwardSearchRequest(Message request)
{
    int budget = request.getBudget() - 1; // -1 for self
    int budgetDeficit = budget >= neighbors->size() ?
                        neighbors->size() : 
                        budget % neighbors->size(); // this isn't right.
                        // each msg needs it's respective budget set 

    request.setBudget(budget - budgetDeficit); 

    if(budgetDeficit >= neighbors.size());


    for(int i = 0; i < budgetDeficit; i++)
    {   
        Q_EMIT(sendMessage(request, pickRandomPeer()));
    }
}

void Mailbox::processCommand(QString cmd)
{
    // TODO: ipmlement datastructure dump signal/slots
    //       for easy print debugging.

    // if(cmd == CMD_PRINT_MSGSTORE)
    // {
    //     qDebug() << "\nMESSAGE STORE:";
    //     qDebug() << msgstore->toString();
    // }

    if(cmd == CMD_PRINT_STATUS)
    {
        qDebug() << "\nOWN STATUS:";
        qDebug() << status->toString();
    }
    else if(cmd == CMD_PRINT_NEIGHBORS)
    {
        qDebug() << "\nNEIGHBORS:";
        foreach(Peer peer, *neighbors)
        {
            qDebug() << "   " << peer.toString();
        }
    }

    // else if(cmd == CMD_PRINT_TABLE)
    // {
    //     qDebug() << "\nROUTING TABLE:";
    //     foreach(QString qstr, routingTable->keys())
    //     {
    //         qDebug() << "   <ORIGIN: " << qstr <<  
    //                         ", PEER: " << nextHop(qstr).toString() << 
    //                         ", DIRECT: " << nextHopIsDirect(qstr) << ">";
    //     }
    // }
}



