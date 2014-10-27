#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#define CMD_PRINT_MSGSTORE ("PRINT_MSGSTORE")
#define CMD_PRINT_STATUS ("PRINT_STATUS")
#define CMD_PRINT_NEIGHBORS ("PRINT_NEIGHBORS")
#define CMD_PRINT_TABLE ("PRINT_TABLE")

#define STATUS_CLOCK_RATE (5000) // 5s in ms
#define ROUTE_CLOCK_RATE (60000) // 60s in ms

#include "Peerster.hh"

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
        void setSelfPeer(Peer*);
        void populateNeighbors();

    signals:
        void sendMessage(Message,Peer);
        void refreshNeighbors(QList<Peer>);
        void processRumor(Message,Peer);
        void processRumorRoute(Message,Peer);
        void processDirectChat(Message);
        void processIncomingStatus(Message,Peer);
        void processBlockRequest(Message);
        void processBlockReply(Message);
        void processSearchRequest(Message);
        void processSearchReply(Message);
        void broadcastRoute();
        void processNeighbor(Peer);
        void monger(Message);
        void broadcast(Message);

    public slots:
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

    private:
        Peerster* peerster;
        QList<Peer>* neighbors;
        QTimer *status_clock, *route_clock;
        QString ID;
        quint32 port, myPortMin, myPortMax;
        Peer *self, *invalid;
        Message* status;
        Peer pickRandomPeer();
        void processCommand(QString);
};

#endif // PEERSTER_MAILBOX_HH

