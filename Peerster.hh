#ifndef PEERSTER_PEERSTER_HH
#define PEERSTER_PEERSTER_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QObject>

#include "ChatDialog.hh"
#include "NetSocket.hh"

class Peerster : public QObject
{
    Q_OBJECT

    public:
        Peerster();
        ~Peerster();

    private:
        ChatDialog* dialog;
        NetSocket* socket;
};

#endif // PEERSTER_PEERSTER_HH