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
        bool isNewRumor(Message);
        bool isNewOrigin(Message);
        void addNewRumor(Message);
        QList<Message> getMessagesInRange(QString,quint32,quint32);
        Message getStatus();
        bool isNextInSeq(Message msg);
        void processIncomingStatus(Message);

    signals:
        void canHelpPeer(Peer,QList<Message>);
        void needHelpFromPeer(Peer);
        void inConsensusWithPeer(Peer);

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* store;
        QMap<QString, quint32>* latest;
};

#endif // PEERSTER_MESSAGESTORE_HH