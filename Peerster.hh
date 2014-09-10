#ifndef PEERSTER_PEERSTER_HH
#define PEERSTER_PEERSTER_HH

#include <unistd.h>

#include <QDialog>
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QKeyEvent>
#include <QVariant>
#include <QUdpSocket>
#include <limits>
#include <QList>
#include <QObject>
#include <QQueue>
#include <QMap>
#include <QTime>

#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Mailbox.hh"
#include "Message.hh"

class ChatDialog;
class NetSocket;
class Mailbox;

class Peerster : public QObject
{
    Q_OBJECT

    public:
        Peerster();
        ~Peerster();
        void run();
        int ID;
        QList<int> findNeighbors();

    private:
        ChatDialog* dialog;
        NetSocket* socket;
        Mailbox* mailbox;
        QList<int> neighbors;
};

#endif // PEERSTER_PEERSTER_HH