#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
{}

Mailbox::~Mailbox()
{}

void Mailbox::gotInboxUpdated()
{
    Message msg = peerster->inboxPop();
    peerster->displayQueuePush(msg);
}

void Mailbox::gotOutboxUpdated()
{
    Message msg = peerster->outboxPop();
    peerster->sendQueuePush(msg);
}

