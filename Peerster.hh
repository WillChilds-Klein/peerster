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

#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Message.hh"
#include "Mailbox.hh"

class ChatDialog;
class NetSocket;
class Mailbox;

class Peerster : public QObject
{
    Q_OBJECT

    public:
        Peerster();
        ~Peerster();
        ChatDialog* getDialog();
        NetSocket* getSocket();
        Mailbox* getMailbox();

    private:
        ChatDialog* dialog;
        NetSocket* socket;
        Mailbox* mailbox;
};

#endif // PEERSTER_PEERSTER_HH