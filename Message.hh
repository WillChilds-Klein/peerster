#ifndef PEERSTER_MESSAGE_HH
#define PEERSTER_MESSAGE_HH

#include "Peerster.hh"

class Message : public QVariantMap
{
    public:
        Message();
        Message(QByteArray* arr);
        ~Message();
        QString toString();
        QByteArray serialize();
};

class Rumor : public Message
{
    public:
        Rumor();
        ~Rumor();
};

class Status : public Message
{
    public:
        Status();
        ~Status();
};

#endif // PEERSTER_MESSAGE_HH