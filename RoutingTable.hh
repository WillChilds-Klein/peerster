#ifndef PEERSTER_ROUTINGTABLE_HH
#define PEERSTER_ROUTINGTABLE_HH

#include "Peerster.hh"

class RoutingTable : public QObject
{
    Q_OBJECT

    public:
        RoutingTable(Peerster*);
        ~RoutingTable();
        QStringList origins();
        Peer get(QString);

    signals:
        void updateGUIOriginsList(QString);

    public slots:
        void gotUpdateTable(Message,Peer);

    private:
        Peerster* peerster;
        QHash<QString,Peer>* table;
};

#endif // PEERSTER_ROUTINGTABLE_HH