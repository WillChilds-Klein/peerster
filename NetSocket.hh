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
        void gotSendMessage(Message);
        void gotStartPeering();
        void gotStopPeering();

    signals:
        void postToInbox(Message);

    private:
        Peerster* peerster;
        QList<int> neighbors;
        int myPortMin, myPortMax, port;
        bool peered;
        int peerPort;
        QList<int> findNeighbors();
        void pickNewPeer();
};

#endif // PEERSTER_NETSOCKET_HH