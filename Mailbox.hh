#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

#define CMD_PRINT_MSGSTORE ("PRINT_MSGSTORE")
#define CMD_PRINT_STATUS ("PRINT_STATUS")

#define CLOCK_RATE (10000) // in ms

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
        void addNeighbor(Peer);
        Peer pickRandomPeer();

    public slots:
        void gotPostToOutbox(Message);
        void gotPostToInbox(Message);
        void gotCanHelpPeer(Peer,QList<Message>);
        void gotNeedHelpFromPeer(Peer);
        void gotInConsensusWithPeer();
        void gotMonger(Message);
        void gotPotentialNewNeighbor(Peer);

    private slots:
        void chime();

    signals:
        void displayMessage(Message);
        void sendMessage(Message,Peer);
        void postToInbox(Message);
        void monger(Message);
        void needHelpFromPeer(Peer);

    private:
        Peerster* peerster;
        QList<Peer>* neighbors;
        MessageStore* msgstore;
        QTimer* clock;
        quint32 ID, localSeqNo;
        quint32 port, myPortMin, myPortMax;
        void processCommand(QString);
};

#endif // PEERSTER_MAILBOX_HH

