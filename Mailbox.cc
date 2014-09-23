#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , neighbors(new QList<Peer>())
    , clock(new QTimer(this))
    , localSeqNo(1)
{
    connect(this, SIGNAL(postToInbox(Message,Peer)), 
        this, SLOT(gotPostToInbox(Message,Peer)));

    connect(this, SIGNAL(monger(Message)),
        this, SLOT(gotMonger(Message)));

    connect(this, SIGNAL(needHelpFromPeer(Peer)),
        this, SLOT(gotNeedHelpFromPeer(Peer)));

    connect(clock, SIGNAL(timeout()), 
        this, SLOT(chime()));

    clock->start(CLOCK_RATE);
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

void Mailbox::populateLocalNeighbors()
{
    QString info;
    for(quint32 i = myPortMin; i <= myPortMax; i++)
    {
        if(i != port)
        {
            info = "127.0.0.1:" + QString::number(i);
            neighbors->append(Peer(info));
        }
    }
}

Peer Mailbox::pickRandomPeer()
{
    return neighbors->at(qrand() % neighbors->size());
}

void Mailbox::gotPostToInbox(Message msg, Peer peer)
{
    if(msg.getType() == TYPE_RUMOR)
    {
        if(msgstore->isNewRumor(msg))
        {
            if(msgstore->isNextInSeq(msg))
            {
                msgstore->addNewRumor(msg);
                qDebug() << "NEW RUMOR FROM : " << peer.toString();
                updateTable(msg, peer);
                Q_EMIT(displayMessage(msg));
                Q_EMIT(monger(msg));
            }
            else if(msg.getOriginID() != ID)
            {
                Q_EMIT(needHelpFromPeer(peer));
            }
        }
        else
        {
            // do nothing
        }
    }
    else
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
    Peer peer = pickRandomPeer();
    Q_EMIT(sendMessage(msg, peer));
}

void Mailbox::gotPotentialNewNeighbor(Peer peer)
{
    if(peer.isWellFormed() && !neighbors->contains(peer))
    {
        neighbors->append(peer);
        // qDebug() << "NEW NEIGHBOR:" << peer.toString();
    }
}

void Mailbox::chime()
{
    Message status = msgstore->getStatus();
    Q_EMIT(monger(status));
}

void Mailbox::gotSendStatusToPeer(Peer peer)
{
    Message status = msgstore->getStatus();
    if(!status.isEmptyStatus())
    {
        Q_EMIT(sendMessage(status, peer));
    }
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


