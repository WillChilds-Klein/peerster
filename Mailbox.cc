#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , inbox(new QQueue<Message>())
    , outbox(new QQueue<Message>())
{}

Mailbox::~Mailbox()
{}

void Mailbox::inboxPush(Message msg)
{}

Message Mailbox::inboxPop()
{}

void Mailbox::outboxPush(Message msg)
{}

Message Mailbox::outboxPop()
{}

void Mailbox::inboxUpdated()
{}

void Mailbox::outboxUpdated()
{} 