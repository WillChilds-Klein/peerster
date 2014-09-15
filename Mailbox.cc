#include "Mailbox.hh"

#define CMD_PRINT_MSGSTORE ("PRINT_MSGSTORE")

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , neighbors(new QList<Peer>())
    , localSeqNo(1)
{
    connect(this, SIGNAL(postToInbox(Message)), 
        this, SLOT(gotPostToInbox(Message)));

    connect(this, SIGNAL(monger(Message)),
        this, SLOT(gotMonger(Message)));

    connect(this, SIGNAL(needHelpFromPeer(Peer)),
        this, SLOT(gotNeedHelpFromPeer(Peer)));
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
    if(port != myPortMin)
    {
        neighbors->append(Peer(port-1));
    }
    if(port != myPortMax)
    {
        neighbors->append(Peer(port+1));
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
                msgstore->addNewRumor(msg);
                Q_EMIT(displayMessage(msg));
                Q_EMIT(monger(msg));
            }
            else
            {
                Q_EMIT(needHelpFromPeer(Peer(msg.getPortOfOrigin())));
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

    processCommand(msg.getText());
}

void Mailbox::gotPostToOutbox(Message msg)
{
    msg.setOriginID(QString::number(ID));
    msg.setSeqNo(localSeqNo);
    localSeqNo++;
    msg.setPortOfOrigin(port);
    Q_EMIT(postToInbox(msg));
}

void Mailbox::gotCanHelpPeer(Peer peer, QList<Message> list)
{
    QList<Message>::iterator i;
    for(i = list.begin(); i != list.end(); ++i)
    {
        Q_EMIT(sendMessage(*i, peer));
    }
}

void Mailbox::gotNeedHelpFromPeer(Peer peer)
{
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

void Mailbox::processCommand(QString cmd)
{
    if(cmd == CMD_PRINT_MSGSTORE)
    {
        qDebug() << msgstore->toString();
    }
}
