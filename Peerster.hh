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

#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Mailbox.hh"
#include "MessageStore.hh"
#include "Message.hh"

#define ID_MAX (10)

#define CHATTEXT_KEY ("ChatText")
#define ORIGINID_KEY ("Origin")
#define SEQNO_KEY ("SeqNo")
#define WANT_KEY ("Want")
#define PORTOFORIGIN_KEY ("Port")

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
        void setPort(quint32);
        quint32 ID, port;
        QList<quint32> findNeighbors();

    private:
        ChatDialog* dialog;
        NetSocket* socket;
        Mailbox* mailbox;
        QList<quint32> neighbors;
};

#endif // PEERSTER_PEERSTER_HH