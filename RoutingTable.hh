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
        bool isNewRumor(Message);

    signals:
        void updateGUIOriginsList(QString);
        void monger(Message);

    public slots:
        void gotUpdateTable(Message,Peer);

    private:
        Peerster* peerster;
        QHash<QString,Peer>* table;
        QHash<QString,quint32>* latest;
        bool isNewOrigin(QString);
};

#endif // PEERSTER_ROUTINGTABLE_HH