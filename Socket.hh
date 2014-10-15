#ifndef PEERSTER_SOCKET_HH
#define PEERSTER_SOCKET_HH

#include "Peerster.hh"

class Peerster;
class Message;
class Peer;

class Socket : public QUdpSocket
{
    Q_OBJECT

    public:
        Socket(Peerster* p);
        ~Socket();
        void setPortRange(quint32,quint32);
        void setNoForward(bool);
        quint32 getPort();
        quint32 getMyPortMin();
        quint32 getMyPortMax();
        qint32 bind();

    public slots:
        void gotReadyRead();
        void gotSendMessage(Message,Peer);

    signals:
        void postToInbox(Message,Peer);
        void potentialNewNeighbor(Peer);

    private:
        Peerster* peerster;
        quint32 port, myPortMin, myPortMax;
        bool noforward;
};

#endif // PEERSTER_SOCKET_HH