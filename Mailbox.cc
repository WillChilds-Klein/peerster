#include "Mailbox.hh"

Mailbox::Mailbox(Peerster* p)
    : peerster(p)
    , ID(p->ID)
    , localSeqNo(1)
    , port(p->port)
    , status(new Message())
    , timer(new QTimer())
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
    msg.setOriginID(QString::number(ID));
    msg.setSeqNo(localSeqNo);
    localSeqNo++;
    msg.setPortOfOrigin(port);
    Q_EMIT(postToInbox(msg));
}

void Mailbox::gotTimeout()
{
    qDebug() << "timeout!!";
    Q_EMIT(stopPeering());
}

