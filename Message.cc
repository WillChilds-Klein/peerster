#include "Message.hh"

Message::Message()
{}

Message::Message(QByteArray* arr)
{
    Message msg;
    QDataStream stream(arr, QIODevice::ReadOnly);
    stream >> msg;

    Message::iterator i;
    for(i = msg.begin(); i != msg.end(); i++)
    {
        this->insert(i.key(), i.value());       
    }
}

Message::~Message()
{}

QByteArray Message::serialize()
{
    QByteArray msgArr;
    QDataStream stream(&msgArr, QIODevice::WriteOnly);
    stream << *this;

    return msgArr;
}