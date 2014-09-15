#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

#define TIMEOUT_MS (1000)

class Peerster;
class Message;
class MessageStore;
class Peer;

class Mailbox : public QObject
{
    Q_OBJECT

    public:
        Mailbox(Peerster*);
        ~Mailbox();
        void setMessageStore(MessageStore*);
        void setPortInfo(quint32,quint32,quint32);
        void setID(quint32);
        void populateNeighbors();
        Peer pickRandomPeer();

    public slots:
        void gotPostToOutbox(Message);
        void gotPostToInbox(Message);
        void gotCanHelpPeer(Peer,QList<Message>);
        void gotNeedHelpFromPeer(Peer);
        void gotInConsensusWithPeer(Peer);
        void gotMonger(Message);

    signals:
        void displayMessage(Message);
        void sendMessage(Message,Peer);
        void postToInbox(Message);
        void monger(Message);

    private:
        Peerster* peerster;
        QList<Peer>* neighbors;
        MessageStore* msgstore;
        quint32 ID, localSeqNo;
        quint32 port, myPortMin, myPortMax;
};

#endif // PEERSTER_MAILBOX_HH

