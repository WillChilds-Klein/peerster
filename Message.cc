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

QString Message::toString()
{
    QString str = "[";
    Message::iterator i;
    for(i = this->begin(); i != this->end(); i++)
    {
        str += "<" + QString(i.key()) + " : " + i.value().toString() + ">, ";
    }
    str += "]\n";
    return str;
}

QByteArray Message::serialize()
{
    QByteArray msgArr;
    QDataStream stream(&msgArr, QIODevice::WriteOnly);
    stream << (*this);

    return msgArr;
}

Rumor::Rumor()
{}

Rumor::~Rumor()
{}

Status::Status()
{}

Status::~Status()
{}