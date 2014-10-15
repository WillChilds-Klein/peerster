#ifndef PEERSTER_ROUTINGTABLE_HH
#define PEERSTER_ROUTINGTABLE_HH

#include "Peerster.hh"

class RoutingTable : public QObject
{
    Q_OBJECT

    public:
        RoutingTable(Peerster*);
        ~RoutingTable();
        Peer nextHop(QString);
        bool nextHopIsDirect(QString);

    signals:
        void updateGUIOriginsList(QStringList);
        void broadcast(Message);
        void monger(Message);

    public slots:
        void gotProcessRumorRoute(Message,Peer);
        void gotSendDirect(Message,QString);
        void gotBroadcastRoute();

    private:
        Peerster* peerster;
        MessageStore* messagestore;
        QHash< QString,QPair<Peer,bool> >* table;
};

#endif // PEERSTER_ROUTINGTABLE_HH