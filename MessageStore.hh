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
        void setGroupConvo(QList<Message>*);
        void setDirectStore(QMap<QString,QList<Message> >*);
        bool isNewRumor(Message);
        bool isNewOrigin(QString);
        bool isNextRumorInSeq(Message);
        void addNewOrigin(QString);
        void addNewChatRumor(Message);
        QList<Message> getMessagesInRange(QString,quint32,quint32);
        Message getStatus();
        QString toString();

        void newDChat(Message);
        void setID(QString);

    signals:
        void helpPeer(Peer,QList<Message>);
        void needHelpFromPeer(Peer);
        void inConsensusWithPeer();

        void processRumorRoute(Message);

        void broadcast(Message);
        void monger(Message);

        void refreshOrigins(QStringList);
        void refreshDirectConvo(QString);
        void refreshGroupConvo();
        // void refreshSharedFiles(QStringList);

        void updateStatus(Message);

    public slots:
        void gotGetDChatHistoryFromOrigin(QString);
        void gotProcessRumor(Message);
        void gotProcessDirectChat(Message);
        void gotProcessIncomingStatus(Message);
        void gotBroadcastRoute();

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* rumorStore;
        QMap< QString, QList<Message> >* directStore;
        QList<Message>* groupConvo;
        QMap<QString, quint32> getLatest();
        Message routeRumor();
        quint32 localSeqNo;
        QString ID;
        QMap< QString, QList<Message> >* histories; 
};

#endif // PEERSTER_MESSAGESTORE_HH