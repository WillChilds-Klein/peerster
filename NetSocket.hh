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
        void gotStartPeering(quint32);
        void gotStopPeering();

    signals:
        void postToInbox(Message);

    private:
        Peerster* peerster;
        QList<quint32> neighbors;
        quint32 myPortMin, myPortMax, port;
        bool peered;
        quint32 peerPort;
        QList<quint32> findNeighbors();
        void pickNewPeer();
};

#endif // PEERSTER_NETSOCKET_HH