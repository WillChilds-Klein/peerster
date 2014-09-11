#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
{
    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    ID = (qrand() % ID_MAX) + 1;
    qDebug() << "Instance ID: "<< ID; 

    // inbound message signal chain
    connect(socket, SIGNAL(postToInbox(Message)), 
        mailbox, SLOT(gotPostToInbox(Message)));

    connect(mailbox, SIGNAL(displayMessage(Message)), 
        dialog, SLOT(gotDisplayMessage(Message)));

    // outbound message signal chain
    connect(dialog, SIGNAL(postToOutbox(Message)), 
        mailbox, SLOT(gotPostToOutbox(Message)));

    connect(mailbox, SIGNAL(sendMessage(Message)), 
        socket, SLOT(gotSendMessage(Message)));

    // peering stuff
    connect(mailbox, SIGNAL(startPeering(quint32)),
        socket, SLOT(gotStartPeering(quint32)));

    connect(mailbox, SIGNAL(stopPeering()),
        socket, SLOT(gotStopPeering()));
}

Peerster::~Peerster()
{}

void Peerster::run()
{   
    // Create an initial chat dialog window
    dialog->show();
}

void Peerster::setPort(quint32 p)
{
    port = p;
}

