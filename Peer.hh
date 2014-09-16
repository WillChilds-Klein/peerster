#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include "Peerster.hh"

class Peer : QHostInfo
{
    Q_OBJECT 
    Q_DECLARE_METATYPE(Peer);

    public:
        Peer(QString);
        ~Peer();
        quint32 getPort();
        QHostAddress getAddress();
        bool isValid();
        QString toString();
        bool operator==(Peer&,Peer&);

    private:
        quint32 port;
        bool valid;

    private slots:
        void hostResolved(QHostInfo);
};

#endif // PEERSTER_PEER_HH