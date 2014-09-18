#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
    , msgstore(new MessageStore(this))
{
    // inbound message signal chain
    connect(socket, SIGNAL(postToInbox(Message,Peer)), 
        mailbox, SLOT(gotPostToInbox(Message,Peer)));
    connect(mailbox, SIGNAL(displayMessage(Message)), 
        dialog, SLOT(gotDisplayMessage(Message)));

    // outbound message signal chain
    connect(dialog, SIGNAL(postToOutbox(Message)), 
        mailbox, SLOT(gotPostToOutbox(Message)));
    connect(mailbox, SIGNAL(sendMessage(Message,Peer)), 
        socket, SLOT(gotSendMessage(Message,Peer)));

    // message processing logic
    connect(msgstore, SIGNAL(canHelpPeer(Peer,QList<Message>)),
        mailbox, SLOT(gotCanHelpPeer(Peer,QList<Message>)));
    connect(msgstore, SIGNAL(needHelpFromPeer(Peer)),
        mailbox, SLOT(gotNeedHelpFromPeer(Peer)));
    connect(msgstore, SIGNAL(inConsensusWithPeer()),
        mailbox, SLOT(gotInConsensusWithPeer()));

    // detect new neighbors
    connect(socket, SIGNAL(potentialNewNeighbor(Peer)),
        mailbox, SLOT(gotPotentialNewNeighbor(Peer)));

    // add peers manually
    connect(dialog, SIGNAL(potentialNewNeighbor(Peer)),
        mailbox, SLOT(gotPotentialNewNeighbor(Peer)));
    connect(dialog, SIGNAL(sendStatusToPeer(Peer)),
        mailbox, SLOT(gotSendStatusToPeer(Peer)));
    connect(this, SIGNAL(potentialNewNeighbor(Peer)),
        mailbox, SLOT(gotPotentialNewNeighbor(Peer)));

    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    ID = QString::number((qrand() % ID_MAX) + 1);
    qDebug() << "Instance ID: "<< ID; 

    myPortMin = 32718 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;

    socket->setPortRange(myPortMin, myPortMax);
    port = socket->bind();
    if(port < 0)
    {
        qDebug() << "Peerster failed to bind to a port!";
        exit(1);
    }

    QString title = "Peerster Instance " + ID + 
        " on port " + QString::number(port);
    dialog->setTitle(title);

    mailbox->setPortInfo(myPortMin, myPortMax, port);
    mailbox->setID(ID);
    mailbox->setMessageStore(msgstore);
    mailbox->populateNeighbors();

    QStringList clargs = QCoreApplication::arguments();
    for(int i = 1; i < clargs.size(); i++)
    {
        Peer peer = Peer(clargs.at(i));
        Q_EMIT(potentialNewNeighbor(peer));
    }
}

Peerster::~Peerster()
{}

void Peerster::run()
{   
    // Create an initial chat dialog window
    dialog->show();
}

