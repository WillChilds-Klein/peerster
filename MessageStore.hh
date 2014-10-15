#ifndef PEERSTER_MESSAGESTORE_HH
#define PEERSTER_MESSAGESTORE_HH

#include "Peerster.hh"

class Message;

// TODO: make MessageStore inherit from type of store
class MessageStore : public QObject
{
    Q_OBJECT

    public:
        MessageStore(Peerster*);
        ~MessageStore();
        void setID(QString);
        void setGroupConvo(QList<Message>*);
        void setRumorStore(QMap< QString,QList<Message> >*);
        void setDirectStore(QMap< QString,QList<Message> >*);
        QString toString();

    signals:
        void helpPeer(Peer,QList<Message>);
        void needHelpFromPeer(Peer);
        void inConsensusWithPeer();
        void monger(Message);
        void updateStatus(Message);
        void sendDirect(Message,QString);
        void processRumorRoute(Message);
        void broadcastRoute();
        void refreshGroupConvo();
        void refreshDirectConvo(QString);
        void refreshOrigins(QStringList);
        void processRumor(Message);
        void processDirectChat(Message);

    public slots:
        void gotCreateChatRumor(QString);
        void gotCreateDirectChat(QString,QString);
        void gotProcessRumor(Message);
        void gotProcessDirectChat(Message);
        void gotProcessIncomingStatus(Message);
        void gotBroadcastRoute();

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* rumorStore;
        QMap< QString, QList<Message> >* directStore;
        QList<Message>* groupConvo;
        quint32 localSeqNo;
        QString ID;
        bool isNewRumor(Message);
        bool isNewOrigin(QString);
        bool isNextRumorInSeq(Message);
        void addRumor(Message);
        void addDirectChat(Message);
        QList<Message> getMessagesInRange(QString,quint32,quint32);
        QMap<QString, quint32> latest();
        Message status();
        Message routeRumor();

};

#endif // PEERSTER_MESSAGESTORE_HH