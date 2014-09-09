#ifndef PEERSTER_MESSAGE_HH
#define PEERSTER_MESSAGE_HH

#include "Peerster.hh"

class Message : public QVariantMap
{
    public:
        Message();
        Message(QByteArray* arr);
        ~Message();
        QByteArray serialize();
};

#endif // PEERSTER_MESSAGE_HH