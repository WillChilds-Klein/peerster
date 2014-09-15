#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , localSeqNo(1)
    , neighbors(new QList<Peer>())
{
    connect(this, SIGNAL(postToInbox(Message)), 
        this, SLOT(gotPostToInbox(Message)));

    connect(this, SIGNAL(monger(Message))),
        this, SLOT(gotMonger(Message));

    connect(this, SIGNAL(monger()),
        this, SLOT(gotMonger()));
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

void Mailbox::setID(quint32 i)
{
    ID = i;
}

void Mailbox::populateNeighbors()
{
    quint32 neighborPort;
    Peer neighborPeer;
    if(port != myPortMin)
    {
        neighborPort = port - 1;
        neighborPeer = Peer::Peer(neighborPort);
        neighbors->append(neighborPeer);
    }
    if(port != myPortMax)
    {
        neighborPort = port + 1;
        neighborPeer = Peer::Peer(neighborPort);
        neighbors->append(neighborPeer);
    }

    // print neighbors.
    qDebug() << "Neighbors: ";
    for(int i = 0; i < neighbors->size(); i++)
    {
        qDebug() << (*neighbors)[i].getPort();
    }
}

Peer Mailbox::pickRandomPeer()
{
    quint32 randIndex = qrand() % neighbors->size();
    return (*neighbors)[randIndex];
}

void Mailbox::gotPostToInbox(Message msg)
{   
    if(msg.typeIsRumor())
    {
        if(msgstore->isNewRumor(msg))
        {
            if(msgstore->isNextInSeq(msg))
            {
                msgstore->addNewMessage(msg);
                Q_EMIT(monger(msg));
                Q_EMIT(displayMessage(msg));
            }
            else
            {
                // do nada
            }
        }
        else
        {
            // do nothing
        }
    }
    else
    {
        msgstore->processIncomingStatus(msg);
    }
}

void Mailbox::gotPostToOutbox(Message msg)
{
    msg.setOriginID(QString::number(ID));
    msg.setSeqNo(localSeqNo);
    localSeqNo++;
    msg.setPortOfOrigin(port);
    Q_EMIT(postToInbox(msg));
}

void Mailbox::gotTimeout()
{
    qDebug() << "timeout!!";
}

void Mailbox::gotCanHelpPeer(Peer p, QList<Message> list)
{
    Message msg;
    for(int i = 0; i < list.size(); i++)
    {
        msg = list[i];
        Q_EMIT(sendMessage(msg, p));
    }
}

void Mailbox::gotNeedHelpFromPeer(Peer p)
{
    Message status = msgstore->getStatus();
    Q_EMIT(sendMessage(status, p));
}

void Mailbox::gotInConsensusWithPeer()
{
    if(qrand() % 2 == 0)
    {
        Q_EMIT(monger(store->getStatus()));
    }
    else
    {
        // halt
    }
}

void Mailbox::gotMonger(Message msg)
{
    Peer peer = pickRandomPeer();
    Q_EMIT(sendMessage(Message, peer));
}


