#ifndef PEERSTER_MESSAGESTORE_HH
#define PEERSTER_MESSAGESTORE_HH

#include "Peerster.hh"

class Message;

class MessageStore : public QObject
{
    Q_OBJECT

    public:
        MessageStore(Peerster*);
        ~MessageStore();
        void setID(QString);
        void setGroupConvo(QList<Message>*);
        void setDirectStore(QMap< QString,QList<Message> >*);
        QString toString();

    signals:
        void helpPeer(Peer,QList<Message>);
        void needHelpFromPeer(Peer);
        void inConsensusWithPeer();
        void processNeighbor(Peer);
        void processRumor(Message,Peer);
        void processRumorRoute(Message,Peer);
        void processDirectChat(Message);
        void monger(Message);
        void updateStatus(Message);
        void sendDirect(Message,QString);
        void sendMessage(Message,Peer);
        void refreshGroupConvo();
        void refreshDirectConvo(QString);
        void refreshOrigins(QStringList);
        void broadcast(Message);

    public slots:
        void gotCreateChatRumor(QString);
        void gotProcessRumorRoute(Message,Peer);
        void gotCreateDirectChat(QString,QString);
        void gotProcessRumor(Message,Peer);
        void gotProcessDirectChat(Message);
        void gotProcessIncomingStatus(Message,Peer);
        void gotSendDirect(Message,QString);
        void gotBroadcastRoute();

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* rumorStore;
        QMap< QString, QList<Message> >* directStore;
        QList<Message>* groupConvo;
        QHash< QString,QPair<Peer,bool> >* table;
        Peer* invalid;
        quint32 localSeqNo;
        QString ID;
        bool isNewRumor(Message);
        bool isNewOrigin(QString);
        bool isNextRumorInSeq(Message);
        bool nextHopIsDirect(QString);
        Peer nextHop(QString);
        void addRumor(Message);
        void addDirectChat(Message);
        QList<Message> getMessagesInRange(QString,quint32,quint32);
        QMap<QString, quint32> latest();
        Message status();
        Message routeRumor();
};

#endif // PEERSTER_MESSAGESTORE_HH