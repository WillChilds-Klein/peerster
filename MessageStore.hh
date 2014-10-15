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
        
        void newDChat(Message);
        void setID(QString);

    signals:
        void canHelpPeer(Peer,QList<Message>);
        void needHelpFromPeer(Peer);
        void inConsensusWithPeer();
        void updateGUIOriginsList(QString);
        void broadcastRoute();

        void updateGUIDChatHistory(QString,QList<Message>);

    public slots:

        void gotGetDChatHistoryFromOrigin(QString);

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* store; 
        QMap<QString, quint32> getLatest();

        QString ID;
        QMap< QString, QList<Message> >* histories; 
};

#endif // PEERSTER_MESSAGESTORE_HH