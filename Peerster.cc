#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
    , msgstore(new MessageStore(this))
{
    // inbound message signal chain
    connect(socket, SIGNAL(postToInbox(Message)), 
        mailbox, SLOT(gotPostToInbox(Message)));
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

    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    ID = (qrand() % ID_MAX) + 1;
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

    QString title = "Peerster Instance " + QString::number(ID) + 
        " on port " + QString::number(port);
    dialog->setTitle(title);

    mailbox->setPortInfo(myPortMin, myPortMax, port);
    mailbox->setID(ID);
    mailbox->setMessageStore(msgstore);
    mailbox->populateNeighbors();
}

Peerster::~Peerster()
{}

void Peerster::run()
{   
    // Create an initial chat dialog window
    dialog->show();
}

