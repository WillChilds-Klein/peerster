#ifndef PEERSTER_ROUTINGTABLE_HH
#define PEERSTER_ROUTINGTABLE_HH

#include "Peerster.hh"

class RoutingTable : public QObject
{
    Q_OBJECT

    public:
        RoutingTable(Peerster*);
        ~RoutingTable();

    signals:
        void sendMessage(Message, Peer);
        void broadcast(Message);

    public slots:
        void gotProcessRumorRoute(Message,Peer);
        void gotSendDirect(Message,QString);
        void broadcastRoute();

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* rumorStore;
        QHash< QString,QPair<Peer,bool> >* table;
        Peer *invalid;
        Peer nextHop(QString);
        bool nextHopIsDirect(QString);
};

#endif // PEERSTER_ROUTINGTABLE_HH