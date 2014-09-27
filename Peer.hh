#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include "Peerster.hh"

class HostInfoHandler;

class Peer
{
    public:
        Peer();
        Peer(QString);
        ~Peer();
        quint16 getPort();
        QHostAddress getAddress();
        bool isWellFormed();
        QString toString();
        bool operator==(Peer);
        bool operator!=(Peer);

    private:
        QList<QHostAddress>* addresses;
        quint16 port;
        bool wellFormed;
};

#endif // PEERSTER_PEER_HH
