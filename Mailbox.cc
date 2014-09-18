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

void Mailbox::populateNeighbors()
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

    // print neighbors.
    qDebug() << "Neighbors: ";
    QList<Peer>::iterator i;
    for(i = neighbors->begin(); i != neighbors->end(); ++i)
    {
        qDebug() << i->getPort();
    }
}

Peer Mailbox::pickRandomPeer()
{
    Peer peer = neighbors->at(qrand() % neighbors->size());

    // keep trying until valid peer is picked.
    while(!peer.isValid())
    {
        peer = neighbors->at(qrand() % neighbors->size());
    }

    return peer;
}

void Mailbox::gotPostToInbox(Message msg, Peer peer)
{   
    qDebug() << "in gotPostToInbox!";
    if(msg.getType() == TYPE_RUMOR)
    {
        if(msgstore->isNewRumor(msg))
        {
            if(msgstore->isNextInSeq(msg))
            {
                msgstore->addNewRumor(msg);
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
    qDebug() << "Helped Peer on port " << peer.getPort() << 
        " by sending " << list.size() << " messages";
}

void Mailbox::gotNeedHelpFromPeer(Peer peer)
{
    Message status = msgstore->getStatus();
    Q_EMIT(sendMessage(status, peer));
    qDebug() << "Need Help from Peer on port " << peer.getPort();
}

void Mailbox::gotInConsensusWithPeer()
{
    qDebug() << "In Consensus with Peer!!";
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
    qDebug() << "MONGER TO PORT" << peer.getPort() << "ON" << peer.getAddress();
    Q_EMIT(sendMessage(msg, peer));
}

void Mailbox::gotPotentialNewNeighbor(Peer peer)
{
    qDebug() << "CONSIDERING" << peer.toString();
    if(!neighbors->contains(peer) && peer.isWellFormed())
    {
        neighbors->append(peer);
        qDebug() << "new neighbor" << peer.toString() << "!";
    }
}

void Mailbox::chime()
{
    Message status = msgstore->getStatus();
    if(!status.isEmptyStatus())
    {
        Q_EMIT(monger(status));
    }
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
        qDebug() << msgstore->toString();
    }
    else if(cmd == CMD_PRINT_STATUS)
    {
        qDebug() << msgstore->getStatus().toString();
    }
    else if(cmd == CMD_PRINT_NEIGHBORS)
    {
        qDebug() << "NEIGHBORS:";
        foreach(Peer peer, *neighbors)
            qDebug() << "   " << peer.toString();
    }
}


