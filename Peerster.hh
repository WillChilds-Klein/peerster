#ifndef PEERSTER_PEERSTER_HH
#define PEERSTER_PEERSTER_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QKeyEvent>
#include <QVariant>
#include <QUdpSocket>
#include <limits>
#include <QList>
#include <QObject>

#include "ChatDialog.hh"
#include "NetSocket.hh"

class ChatDialog;
class NetSocket;
class Peerster : public QObject
{
    Q_OBJECT

    public:
        Peerster();
        ~Peerster();
        ChatDialog* getDialog();
        NetSocket* getSocket();

    private:
        ChatDialog* dialog;
        NetSocket* socket;
};

#endif // PEERSTER_PEERSTER_HH