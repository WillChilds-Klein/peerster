#ifndef PEERSTER_MESSAGESTORE_HH
#define PEERSTER_MESSAGESTORE_HH

#include "Peerster.hh"

class Message;

class MessageStore : QObject
{
    Q_OBJECT

    public:
        MessageStore();
        ~MessageStore();
        bool isNewRumor(Message);
        void addNewMessage(Message);
        QList<Message> getMessagesInRange(QString,quint32,quint32);

    private:
        QMap<QString, QList<Message> >* store;
        QMap<QString, quint32>* latest;
};

#endif // PEERSTER_MESSAGESTORE_HH