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
        bool isNewRumor(Message);
        bool isNewOrigin(QString);
        bool isNextRumorInSeq(Message);
        void addNewOrigin(QString);
        void addNewChatRumor(Message);
        QList<Message> getMessagesInRange(QString,quint32,quint32);
        Message getStatus();
        void processIncomingStatus(Message,Peer);
        QString toString();

    signals:
        void canHelpPeer(Peer,QList<Message>);
        void needHelpFromPeer(Peer);
        void inConsensusWithPeer();
        void updateGUIOriginsList(QString);
        void broadcastRoute();

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* store; 
        QMap<QString, quint32> getLatest();
};

#endif // PEERSTER_MESSAGESTORE_HH