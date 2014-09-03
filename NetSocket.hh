#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>

class NetSocket : public QUdpSocket
{
    Q_OBJECT

    public:
        NetSocket();
        ~NetSocket();
        bool bind();

    private:
        int myPortMin, myPortMax;
};

#endif // PEERSTER_NETSOCKET_HH