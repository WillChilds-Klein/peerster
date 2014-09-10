#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

#define TIMEOUT_MS (1000)

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
        void gotTimeout();

    signals:
        void displayMessage(Message);
        void sendMessage(Message);
        void postToInbox(Message);
        void startPeering();
        void stopPeering();

    private:
        Peerster* peerster;
        QQueue<Message>* inbox;
        QQueue<Message>* outbox;
        QTimer* timer;
        int ID;
        int localSeqNo;
};

#endif // PEERSTER_MAILBOX_HH

