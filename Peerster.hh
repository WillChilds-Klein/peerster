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

#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Message.hh"

class ChatDialog;
class NetSocket;
class Message;

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