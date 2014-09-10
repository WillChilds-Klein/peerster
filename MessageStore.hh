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
        bool isNewMessage(Message);

    private:
        bool isStatus;
};

#endif // PEERSTER_MESSAGESTORE_HH