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
        qint32 bind();

    signals:
        void postToInbox(Message,Peer);
        void processNeighbor(Peer);

    public slots:
        void gotSendMessage(Message,Peer);

    private slots:
        void gotReadyRead();

    private:
        Peerster* peerster;
        quint32 port, myPortMin, myPortMax;
        bool noforward;
};

#endif // PEERSTER_SOCKET_HH