#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include "Peerster.hh"

class Peerster;
class Message;
class Peer;

class NetSocket : public QUdpSocket
{
    Q_OBJECT

    public:
        NetSocket(Peerster* p);
        ~NetSocket();
        void setPortRange(quint32,quint32);
        quint32 getPort();
        quint32 getMyPortMin();
        quint32 getMyPortMax();
        qint32 bind();

    public slots:
        void gotReadyRead();
        void gotSendMessage(Message,Peer);

    signals:
        void postToInbox(Message);

    private:
        Peerster* peerster;
        quint32 port, myPortMin, myPortMax;
};

#endif // PEERSTER_NETSOCKET_HH