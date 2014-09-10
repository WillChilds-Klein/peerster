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
        void displayQueuePush(Message msg);
        Message displayQueuePop();
        bool displayQueueIsEmpty();
        void inboxPush(Message msg);
        Message inboxPop();
        bool inboxIsEmpty();
        void outboxPush(Message msg);
        Message outboxPop();
        bool outboxIsEmpty();
        void sendQueuePush(Message msg);
        Message sendQueuePop();
        bool sendQueueIsEmpty();

    signals:
        void inboxUpdated();
        void displayQueueUpdated();
        void outboxUpdated();
        void sendMessage();

    private:
        ChatDialog* dialog;
        NetSocket* socket;
        Mailbox* mailbox;
        QQueue<Message>* inbox;
        QQueue<Message>* displayQueue;
        QQueue<Message>* outbox;
        QQueue<Message>* sendQueue;
};

#endif // PEERSTER_PEERSTER_HH