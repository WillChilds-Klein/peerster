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
#include <QTimer>
#include <QListIterator>
#include <QThread>
#include <typeinfo>
#include <QHostInfo>
#include <QHostAddress>

#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Mailbox.hh"
#include "MessageStore.hh"
#include "Message.hh"
#include "Peer.hh"

#define ID_MAX (1000)

#define CHATTEXT_KEY ("ChatText")
#define ORIGINID_KEY ("Origin")
#define SEQNO_KEY ("SeqNo")
#define WANT_KEY ("Want")
#define PEEROFORIGIN_KEY ("Peer")
#define TYPE_KEY ("Type")
#define TYPE_RUMOR ("Rumor")
#define TYPE_STATUS ("Status")

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
        QList<quint32> findNeighbors();

    private:
        qint32 port;
        quint32 myPortMin, myPortMax;
        QString ID;
        ChatDialog* dialog;
        NetSocket* socket;
        Mailbox* mailbox;
        MessageStore* msgstore;
        QList<quint32> neighbors;
};

#endif // PEERSTER_PEERSTER_HH