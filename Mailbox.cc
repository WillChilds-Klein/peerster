#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , inbox(new QQueue<Message>())
    , outbox(new QQueue<Message>())
{}

Mailbox::~Mailbox()
{}

void Mailbox::gotPostToInbox(Message msg)
{
    inbox->enqueue(msg);

    Message out = inbox->dequeue();
    Q_EMIT(displayMessage(msg));
}

void Mailbox::gotPostToOutbox(Message msg)
{
    outbox->enqueue(msg);

    Message out = outbox->dequeue();
    Q_EMIT(sendMessage(out));
}

