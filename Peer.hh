#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include "Peerster.hh"

class Peer
{
    public:
        Peer();
        Peer(quint32);
        ~Peer();
        quint32 getPort();
        QHostAddress getHost();

    private:
        quint32 port;
        QHostAddress* host;
};

#endif // PEERSTER_PEER_HH