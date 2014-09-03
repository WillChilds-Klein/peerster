#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include "Peerster.hh"

class Peerster;
class Message;

class NetSocket : public QUdpSocket
{
    Q_OBJECT

    public:
        NetSocket(Peerster* p);
        ~NetSocket();
        bool bind();
        void send(Message msg);

    private:
        Peerster* peerster;
        int myPortMin, myPortMax;
};

#endif // PEERSTER_NETSOCKET_HH