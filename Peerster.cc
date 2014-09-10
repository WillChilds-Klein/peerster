#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
    , mailbox(new Mailbox(this))
    , inbox(new QQueue<Message>())
    , displayQueue(new QQueue<Message>())
    , outbox(new QQueue<Message>())
    , sendQueue(new QQueue<Message>())
{
    // inbound message signal chain
    connect(socket, SIGNAL(readyRead()), socket, SLOT(gotReadyRead()));
    
    connect(this, SIGNAL(inboxUpdated()), mailbox, SLOT(gotInboxUpdated()));

    connect(this, SIGNAL(displayQueueUpdated()), dialog, SLOT(gotDisplayQueueUpdated()));

    // outbound message signal chain
    connect(this, SIGNAL(outboxUpdated()), mailbox, SLOT(gotOutboxUpdated()));

    connect(this, SIGNAL(sendMessage()), socket, SLOT(gotSendMessage()));
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

void Peerster::displayQueuePush(Message msg)
{
    displayQueue->enqueue(msg);
    qDebug() << "displayQueue pushed onto!";
    qDebug() << "MSG: " + msg.toString();
    Q_EMIT(displayQueueUpdated());
}

Message Peerster::displayQueuePop()
{
    return displayQueue->dequeue();
}

bool Peerster::displayQueueIsEmpty()
{
    return displayQueue->isEmpty();
}

void Peerster::inboxPush(Message msg)
{
    inbox->enqueue(msg);
    qDebug() << "inbox pushed onto!";
    Q_EMIT(inboxUpdated());
}

Message Peerster::inboxPop()
{
    return inbox->dequeue();
}

bool Peerster::inboxIsEmpty()
{
    return inbox->isEmpty();
}

void Peerster::outboxPush(Message msg)
{
    outbox->enqueue(msg);
    qDebug() << "outbox pushed onto!";
    Q_EMIT(outboxUpdated());
}

Message Peerster::outboxPop()
{
    return outbox->dequeue();
}

bool Peerster::outboxIsEmpty()
{
    return outbox->isEmpty();
}

void Peerster::sendQueuePush(Message msg)
{
    sendQueue->enqueue(msg);
    qDebug() << "sendQueue pushed onto!";
    Q_EMIT(sendMessage());
}

Message Peerster::sendQueuePop()
{
    return sendQueue->dequeue();
}

bool Peerster::sendQueueIsEmpty()
{
    return sendQueue->isEmpty();
}

