#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

class Peerster;
class Message;

class Mailbox : QObject
{
    Q_OBJECT

    public:
        Mailbox(Peerster* p);
        ~Mailbox();
        void inboxPush(Message msg);
        Message inboxPop();
        void outboxPush(Message msg);
        Message outboxPop();

    public slots:
        void inboxUpdated();
        void outboxUpdated();

    private:
        Peerster* peerster;
        QQueue<Message>* inbox;
        QQueue<Message>* outbox;
};

#endif // PEERSTER_MAILBOX_HH

