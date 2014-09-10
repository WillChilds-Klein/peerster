#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

class Peerster;
class Message;

class Mailbox : public QObject
{
    Q_OBJECT

    public:
        Mailbox(Peerster* p);
        ~Mailbox();

    public slots:
        void gotPostToOutbox(Message msg);
        void gotPostToInbox(Message msg);

    signals:
        void displayMessage(Message);
        void sendMessage(Message);

    private:
        Peerster* peerster;
        QQueue<Message>* inbox;
        QQueue<Message>* outbox;
};

#endif // PEERSTER_MAILBOX_HH

