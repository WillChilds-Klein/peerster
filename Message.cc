#include "Message.hh"

Message::Message()
    : isStatus(true)
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

bool Message::typeIsStatus()
{
    return isStatus;
}

void Message::setText(QString qstr)
{
    insert(CHATTEXT_KEY, qstr);
}

void Message::setOriginID(QString qstr)
{
    insert(ORIGINID_KEY, qstr);
}

void Message::setSeqNo(quint32 seqno)
{
    insert(SEQNO_KEY, seqno);
}

