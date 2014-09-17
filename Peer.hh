#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include "Peerster.hh"

// class HostInfoHandler;

class Peer
{
    public:
        Peer();
        Peer(QString);
        ~Peer();
        quint32 getPort();
        QHostAddress getAddress();
        // bool isValid();
        QString toString();
        // void newHostInfo(QHostInfo);
        bool operator==(Peer);

    private:
        QHostInfo* info;
        // HostInfoHandler* handler;
        quint32 port;
        // bool valid;
};

// class HostInfoHandler : public QObject
// {
//     Q_OBJECT

//     public:
//         HostInfoHandler(Peer*);

//     signals:
//         void hostResolved(QHostInfo);

//     private slots:
//         void gotHostResolved(QHostInfo);

//     private:
//         Peer* peer;
// };

#endif // PEERSTER_PEER_HH