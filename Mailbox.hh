#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"
#include "Message.hh"
#include "MessageStore.hh"

#define TIMEOUT_MS (1000)

class Peerster;
class Message;
class MessageStore;

class Mailbox : public QObject
{
    Q_OBJECT

    public:
        Mailbox(Peerster* p);
        ~Mailbox();

    public slots:
        void gotPostToOutbox(Message);
        void gotPostToInbox(Message);
        void gotTimeout();

    signals:
        void displayMessage(Message);
        void sendMessage(Message);
        void postToInbox(Message);
        void startPeering(quint32);
        void stopPeering();

    private:
        Peerster* peerster;
        QQueue<Message>* inbox;
        QQueue<Message>* outbox;
        MessageStore* store;
        Message* status;
        QTimer* timer;
        quint32 ID, localSeqNo, port;
};

#endif // PEERSTER_MAILBOX_HH

