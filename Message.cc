#include "Message.hh"

Message::Message()
{
    insert(TYPE_KEY, TYPE_RUMOR);
}

Message::Message(QByteArray* arr)
{
    // TODO: add robustness code to make sure arr deserializes
    // to well-formed Message object.
    Message msg;
    QDataStream stream(arr, QIODevice::ReadOnly);
    stream >> msg;

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
    QString str;
    QVariantMap::iterator i;
    if(getType() == TYPE_RUMOR)
    {
        str += "[";
        for(i = this->begin(); i != this->end(); ++i)
        {
            str += "<" + i.key() + ": " + i.value().toString() + ">,";
        }
        str += "]";
    }
    else
    {
        str += "<Want: ";
        QVariantMap wantMap = value(WANT_KEY).toMap();
        for(i = wantMap.begin(); i != wantMap.end(); ++i)
        {
            str += "<" + i.key() + ": " + i.value().toString() + ">,";
        }
        str += ">";
    }

    return str;
}

QByteArray Message::serialize()
{
    QByteArray msgArr;
    QDataStream stream(&msgArr, QIODevice::WriteOnly);
    stream << (*this);

    return msgArr;
}

void Message::setType(QString str)
{
    insert(TYPE_KEY, str);
}

void Message::setPortOfOrigin(quint32 p)
{
    insert(PORTOFORIGIN_KEY, p);
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

void Message::setWantMap(QVariantMap qvm)
{
    insert(WANT_KEY, qvm);
}

QString Message::getType()
{
    return value(TYPE_KEY).toString();
}

quint32 Message::getPortOfOrigin()
{
    return value(PORTOFORIGIN_KEY).toInt();
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

QVariantMap Message::getWantMap()
{
    return value(WANT_KEY).toMap();
}
