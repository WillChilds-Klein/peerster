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
        int getPort();
        bool bind();
        void send(Message msg);

    public slots:
        void gotReadyRead();
        void sendMessage();

    private:
        Peerster* peerster;
        int myPortMin, myPortMax, port;
};

#endif // PEERSTER_NETSOCKET_HH