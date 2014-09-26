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
#include <QPushButton>
#include <QCoreApplication>
#include <QHash>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>

#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Mailbox.hh"
#include "MessageStore.hh"
#include "DChatStore.hh"
#include "RoutingTable.hh"
#include "Message.hh"
#include "Peer.hh"

#define ID_MAX (1000)

#define DCHAT_HOP_LIMIT (10)

#define KEY_CHATTEXT ("ChatText")
#define KEY_ORIGINID ("Origin")
#define KEY_SEQNO ("SeqNo")
#define KEY_WANT ("Want")
#define KEY_TYPE ("Type")
#define KEY_DEST ("Dest")
#define KEY_HOPLIMIT ("HopLimit")
#define TYPE_RUMOR_CHAT ("ChatRumor")
#define TYPE_RUMOR_ROUTE ("RouteRumor")
#define TYPE_STATUS ("Status")
#define TYPE_DIRECT_CHAT ("DirectChat")
#define TYPE_NONE ("None")

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

    private:
        qint32 port;
        quint32 myPortMin, myPortMax;
        QString ID;
        ChatDialog* dialog;
        NetSocket* socket;
        Mailbox* mailbox;
        RoutingTable* table;
        MessageStore* msgstore;
        DChatStore* dchatstore;
        QList<quint32> neighbors;
};

#endif // PEERSTER_PEERSTER_HH