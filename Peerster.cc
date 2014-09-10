#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
{
    qsrand(time(NULL));
    ID = (qrand() % ID_MAX) + 1;

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
    connect(mailbox, SIGNAL(startPeering()),
        socket, SLOT(gotStartPeering()));

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

