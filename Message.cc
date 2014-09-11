#include "Message.hh"

Message::Message()
    : isStatus(false)
{}

Message::Message(QByteArray* arr)
{
    // add robustness code to make sure arr deserializes
    // to well-formed Message object.
    Message msg;
    QDataStream stream(arr, QIODevice::ReadOnly);
    stream >> msg;

    isStatus = msg.typeIsStatus();

    Message::iterator i;
    for(i = msg.begin(); i != msg.end(); i++)
    {
        insert(i.key(), i.value());       
    }
}

Message::~Message()
{}

QString Message::toString()
{
    QString str = "size = " + QString::number(size()) +" [";
    foreach(QString e, this->keys())
    {
        str += "<" + e + " : " + value(e).toString() + ">, ";
    }
    str += "]";
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

void Message::setPortOfOrigin(quint32 p)
{
    insert(PORTOFORIGIN_KEY, p);
}

QString Message::getText()
{
    return value(CHATTEXT_KEY).toString();
}

QString Message::getOriginID()
{
    return value(ORIGINID_KEY).toString();
}

quint32 Message::getSeqNo()
{
    return value(SEQNO_KEY).toInt();
}

quint32 Message::getPortOfOrigin()
{
    return value(PORTOFORIGIN_KEY).toInt();
}

