#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , neighbors(new QList<Peer>())
    , status_clock(new QTimer(this))
    , route_clock(new QTimer(this))
    , localSeqNo(1)
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

void Mailbox::setRoutingTable(RoutingTable* r)
{
    table = r;
}

void Mailbox::setPortInfo(quint32 min, quint32 max, quint32 p)
{
    myPortMin = min;
    myPortMax = max;
    port = p;
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
    /** /
    QString info;
    for(quint32 i = myPortMin; i <= myPortMax; i++)
      {
        if(i != port)
        {
            info = "127.0.0.1:" + QString::number(i);
            neighbors->append(Peer(info));
        }
    }
    /**/
    if(port != myPortMin)
    {
        neighbors->append(Peer("127.0.0.1:" + QString::number(port-1)));
    }
    if(port != myPortMax)
    {
        neighbors->append(Peer("127.0.0.1:" + QString::number(port+1)));
    }

    qDebug() << "NEIGHBORS:";
    foreach(Peer peer, *neighbors)
    {
        qDebug() << "   " << peer.toString();
    }
}

void Mailbox::broadcastRoute()
{
    Message route = routeRumor();
    foreach(Peer peer, *neighbors)
    {
        sendMessage(route, peer);
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
        if(msgstore->isNewRumor(msg))
        {
            if(msgstore->isNextInSeq(msg))
            {
                if(msg.getOriginID() != ID)
                {
                    updateTable(msg, peer);
                    Q_EMIT(monger(msg));
                }
                if(msg.getType() == TYPE_RUMOR_CHAT)
                {
                    Q_EMIT(displayMessage(msg));
                    msgstore->addNewRumor(msg);
                }
            }
            else if(msg.getOriginID() != ID)
            {
                if(msg.getType() == TYPE_RUMOR_ROUTE)
                {
                    updateTable(msg, peer);
                    Q_EMIT(monger(msg));
                }
                else if(msg.getType() == TYPE_RUMOR_CHAT)
                {
                    Q_EMIT(needHelpFromPeer(peer));
                }
            }
        }
        else
        {
            // do nothing
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
    msg.setSeqNo(localSeqNo);
    localSeqNo++;
    Peer peer = Peer("127.0.0.1:" + QString::number(port));
    Q_EMIT(postToInbox(msg, peer));
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
    else
    {
        // halt
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
    if(peer.isWellFormed() && !neighbors->contains(peer))
    {
        neighbors->append(peer);
        // qDebug() << "NEW NEIGHBOR:" << peer.toString();
    }
}

void Mailbox::status_chime()
{
    Message status = msgstore->getStatus();
    Q_EMIT(monger(status));
}

void Mailbox::route_chime()
{
    Message route = routeRumor();
    Q_EMIT(monger(route));
    qDebug() << "MONGER ROUTE: " << route.toString();
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


