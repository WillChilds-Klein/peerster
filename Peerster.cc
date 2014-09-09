#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
{
    connect(socket, SIGNAL(readyRead()), socket, SLOT(gotReadyRead()));
}

Peerster::~Peerster()
{}

ChatDialog* Peerster::getDialog() 
{
    return this->dialog;
}

NetSocket* Peerster::getSocket()
{
    return this->socket;
}

Mailbox* Peerster::getMailbox()
{
    return this->mailbox;
}