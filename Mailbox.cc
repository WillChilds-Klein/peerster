#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , timer(new QTimer())
    , ID(p->ID)
    , localSeqNo(1)
{
    connect(this, SIGNAL(postToInbox(Message)), 
        this, SLOT(gotPostToInbox(Message)));
    connect(timer, SIGNAL(timeout()), 
        this, SLOT(gotTimeout()));
}

Mailbox::~Mailbox()
{}

void Mailbox::gotPostToInbox(Message msg)
{
    Q_EMIT(displayMessage(msg));
}

void Mailbox::gotPostToOutbox(Message msg)
{
    msg.setOriginID(QString(ID));
    msg.setSeqNo(localSeqNo);
    localSeqNo++;
    Q_EMIT(postToInbox(msg));
}

void Mailbox::gotTimeout()
{
    Q_EMIT(stopPeering());
}
