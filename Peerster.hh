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
#include <QPair>

#include "GUI.hh"
#include "Socket.hh"
#include "Mailbox.hh"
#include "MessageStore.hh"
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
#define KEY_LASTIP ("LastIP")
#define KEY_LASTPORT ("LastPort")
#define TYPE_RUMOR_CHAT ("ChatRumor")
#define TYPE_RUMOR_ROUTE ("RouteRumor")
#define TYPE_STATUS ("Status")
#define TYPE_DIRECT_CHAT ("DirectChat")
#define TYPE_NONE ("None")
#define SWITCH_NOFORWARD ("-noforward")

class GUI;
class Socket;
class Mailbox;
class MessageStore;
class RoutingTable;

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
        GUI* gui;
        Socket* socket;
        Mailbox* mailbox;
        MessageStore* messagestore;
        RoutingTable* table;
        QList<quint32> neighbors;
        bool noforward;
};

#endif // PEERSTER_PEERSTER_HH