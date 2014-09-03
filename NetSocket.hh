#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>

#include "Peerster.hh"

class Peerster;
class NetSocket : public QUdpSocket
{
    Q_OBJECT

    public:
        NetSocket(Peerster* p);
        ~NetSocket();
        bool bind();

    private:
        Peerster* peerster;
        int myPortMin, myPortMax;
};

#endif // PEERSTER_NETSOCKET_HH