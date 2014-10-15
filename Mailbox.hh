#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

#define CMD_PRINT_MSGSTORE ("PRINT_MSGSTORE")
#define CMD_PRINT_STATUS ("PRINT_STATUS")
#define CMD_PRINT_NEIGHBORS ("PRINT_NEIGHBORS")
#define CMD_PRINT_TABLE ("PRINT_TABLE")

#define STATUS_CLOCK_RATE (5000) // 5s in ms
#define ROUTE_CLOCK_RATE (60000) // 60s in ms

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
        void setPortInfo(quint32,quint32,quint32);
        void setID(QString);
        void populateNeighbors();
        Peer pickRandomPeer();

    public slots:
        void gotPostToOutbox(Message);
        void gotPostToInbox(Message,Peer);
        void gotProcessNeighbor(Peer);
        void gotHelpPeer(Peer,QList<Message>);
        void gotNeedHelpFromPeer(Peer);
        void gotInConsensusWithPeer();
        void gotUpdateStatus(Message);
        void gotMonger(Message);
        void gotBroadcast(Message);

    private slots:
        void status_chime();
        void route_chime();
        void gotSendStatusToPeer(Peer);

    signals:
        void postToInbox(Message,Peer);
        void sendMessage(Message,Peer);
        void refreshNeighbors(QList<Peer>);
        void monger(Message);
        void broadcast(Message);
        void broadcastRoute();

    private:
        Peerster* peerster;
        QList<Peer>* neighbors;
        QTimer *status_clock, *route_clock;
        QString ID;
        quint32 port, myPortMin, myPortMax;
        Peer *self, *invalid;
        Message status;
        void processCommand(QString);
};

#endif // PEERSTER_MAILBOX_HH

