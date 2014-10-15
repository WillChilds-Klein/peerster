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
        void gotCanHelpPeer(Peer,QList<Message>);
        void gotNeedHelpFromPeer(Peer);
        void gotInConsensusWithPeer();
        void gotMonger(Message);
        void gotPotentialNewNeighbor(Peer);
        void gotBroadcast(Message);

        // to implement
        void gotUpdateStatus(Message);

    private slots:
        void status_chime();
        void route_chime();
        void gotSendStatusToPeer(Peer);

    signals:
        void refreshGroupConvo(Message);
        void sendMessage(Message,Peer);
        void postToInbox(Message,Peer);
        void monger(Message);
        void needHelpFromPeer(Peer);
        void updateTable(Message,Peer);
        void updateGUINeighbors(QList<Peer>);
        void broadcast(Message);
        void broadcastRoute();
        void updateGUIOriginsList(QString);

        // signals to use, still need to implement slots
        void processRumorRoute(Message);
        void refreshNeighbors(QStringList);

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

