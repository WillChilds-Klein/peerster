#include "Message.hh"

Message::Message()
    : wellFormed(true)
{
    setType(TYPE_RUMOR);
}

Message::Message(QByteArray* arr)
    : wellFormed(true)
{
    QVariantMap map;
    QDataStream stream(arr, QIODevice::ReadOnly);
    stream >> map;

    if(stream.status() == QDataStream::Ok)
    {
        QVariantMap::iterator i;
        for(i = map.begin(); i != map.end(); i++)
        {
            insert(i.key(), i.value());       
        }

        if(contains(WANT_KEY))
        {
            setType(TYPE_STATUS);
        }
        else if(contains(CHATTEXT_KEY) && contains(ORIGINID_KEY) 
            && contains(SEQNO_KEY))
        {
            setType(TYPE_RUMOR);
        }
    }
    else
    {
        wellFormed = false;
        qDebug() << "MALFORMED MESSAGE";
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
    else if(getType() == TYPE_STATUS)
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

QByteArray Message::toSerializedQVMap()
{
    QByteArray msgArr;
    QDataStream stream(&msgArr, QIODevice::WriteOnly);

    QVariantMap map;
    if(getType() == TYPE_RUMOR)
    {
        map.insert(CHATTEXT_KEY, value(CHATTEXT_KEY));
        map.insert(ORIGINID_KEY, value(ORIGINID_KEY));
        map.insert(SEQNO_KEY, value(SEQNO_KEY));
    }
    else if(getType() == TYPE_STATUS)
    {
        map.insert(WANT_KEY, value(WANT_KEY));
    }

    stream << map;

    return msgArr;
}

bool Message::isWellFormed()
{
    bool isStatus, isRumor;
    isStatus = contains(WANT_KEY);
    isRumor = (contains(CHATTEXT_KEY) && contains(ORIGINID_KEY) 
        && contains(SEQNO_KEY));
    if(isStatus || isRumor)
    {
        return true;
    }

    return false;
}

bool Message::isEmptyStatus()
{
    return value(WANT_KEY).toMap().size() == 0;
}

void Message::setType(QString str)
{
    insert(TYPE_KEY, str);
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
