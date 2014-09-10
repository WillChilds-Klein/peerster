#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
{
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
}

Peerster::~Peerster()
{}

void Peerster::run()
{
    // Create an initial chat dialog window
    dialog->show();

    // Create a UDP network socket
    if (!socket->bind())
    {
        qDebug() << "Peerster failed to bind!";
        exit(1);
    }
}

