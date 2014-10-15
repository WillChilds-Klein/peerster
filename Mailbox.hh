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
class DChatStore;
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
        Message routeRumor();
        void populateNeighbors();
        Peer pickRandomPeer();

    public slots:
        void gotPostToOutbox(Message);
        void gotPostToInbox(Message,Peer);
        void gotCanHelpPeer(Peer,QList<Message>);
        void gotNeedHelpFromPeer(Peer);
        void gotInConsensusWithPeer();
        void gotMonger(Message);
        void gotPotentialNewNeighbor(Peer);
        void gotBroadcast(Message);
        void gotBroadcastRoute();

    private slots:
        void status_chime();
        void route_chime();
        void gotSendStatusToPeer(Peer);

    signals:
        void displayMessage(Message);
        void sendMessage(Message,Peer);
        void postToInbox(Message,Peer);
        void monger(Message);
        void needHelpFromPeer(Peer);
        void updateTable(Message,Peer);
        void updateGUINeighbors(QList<Peer>);
        void broadcast(Message);
        void broadcastRoute();
        void updateGUIOriginsList(QString);

    private:
        Peerster* peerster;
        QList<Peer>* neighbors;
        MessageStore* msgstore;
        DChatStore* dchatstore;
        QHash< QString,QPair<Peer,bool> >* routingTable;
        QTimer *status_clock, *route_clock;
        QString ID;
        quint32 localSeqNo;
        quint32 port, myPortMin, myPortMax;
        Peer *self, *invalid;
        void processCommand(QString);
        void processRumorRouteInfo(Message,Peer);
        Peer nextHop(QString);
        bool nextHopIsDirect(QString);
};

#endif // PEERSTER_MAILBOX_HH

