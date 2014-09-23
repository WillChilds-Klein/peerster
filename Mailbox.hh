#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

#define CMD_PRINT_MSGSTORE ("PRINT_MSGSTORE")
#define CMD_PRINT_STATUS ("PRINT_STATUS")
#define CMD_PRINT_NEIGHBORS ("PRINT_NEIGHBORS")
#define CMD_PRINT_TABLE ("PRINT_TABLE")

#define CLOCK_RATE (5000) // in ms

class Peerster;
class MessageStore;
class RoutingTable;
class Message;
class Peer;

class Mailbox : public QObject
{
    Q_OBJECT

    public:
        Mailbox(Peerster*);
        ~Mailbox();
        void setMessageStore(MessageStore*);
        void setRoutingTable(RoutingTable*);
        void setPortInfo(quint32,quint32,quint32);
        void setID(QString);
        void populateLocalNeighbors();
        Peer pickRandomPeer();

    public slots:
        void gotPostToOutbox(Message);
        void gotPostToInbox(Message,Peer);
        void gotCanHelpPeer(Peer,QList<Message>);
        void gotNeedHelpFromPeer(Peer);
        void gotInConsensusWithPeer();
        void gotMonger(Message);
        void gotPotentialNewNeighbor(Peer);

    private slots:
        void chime();
        void gotSendStatusToPeer(Peer);

    signals:
        void displayMessage(Message);
        void sendMessage(Message,Peer);
        void postToInbox(Message,Peer);
        void monger(Message);
        void needHelpFromPeer(Peer);
        void updateTable(Message,Peer);

    private:
        Peerster* peerster;
        QList<Peer>* neighbors;
        MessageStore* msgstore;
        RoutingTable* table;
        QTimer* clock;
        QString ID;
        quint32 localSeqNo;
        quint32 port, myPortMin, myPortMax;
        void processCommand(QString);
};

#endif // PEERSTER_MAILBOX_HH

