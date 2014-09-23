#include "Message.hh"

Message::Message()
    : wellFormed(true)
{
    setType(TYPE_NONE);
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

        if(contains(KEY_WANT))
        {
            setType(TYPE_STATUS);
        }
        else if(contains(KEY_ORIGINID) && contains(KEY_SEQNO) 
            && contains(KEY_CHATTEXT))
        {
            setType(TYPE_RUMOR_CHAT);
        }
        else if(contains(KEY_ORIGINID) && contains(KEY_SEQNO)
            && !contains(KEY_CHATTEXT))
        {
            setType(TYPE_RUMOR_ROUTE);
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
    if(getType() == TYPE_RUMOR_CHAT || getType() == TYPE_RUMOR_ROUTE)
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
        str += "[Want: ";
        QVariantMap wantMap = value(KEY_WANT).toMap();
        for(i = wantMap.begin(); i != wantMap.end(); ++i)
        {
            str += "<" + i.key() + ": " + i.value().toString() + ">,";
        }
        str += "]";
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
    if(getType() == TYPE_RUMOR_CHAT)
    {
        map.insert(KEY_CHATTEXT, value(KEY_CHATTEXT));
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_SEQNO, value(KEY_SEQNO));
    }
    else if(getType() == TYPE_RUMOR_ROUTE)
    {
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_SEQNO, value(KEY_SEQNO));
    }
    else if(getType() == TYPE_STATUS)
    {
        map.insert(KEY_WANT, value(KEY_WANT));
    }

    stream << map;

    return msgArr;
}

bool Message::isWellFormed()
{
    bool isStatus, isRumor;
    isStatus = contains(KEY_WANT);
    isRumor = (contains(KEY_ORIGINID) && contains(KEY_SEQNO));
    if(isStatus || isRumor)
    {
        return true;
    }

    return false;
}

bool Message::isEmptyStatus()
{
    return value(KEY_WANT).toMap().size() == 0;
}

void Message::setType(QString str)
{
    insert(KEY_TYPE, str);
}

void Message::setText(QString qstr)
{
    insert(KEY_CHATTEXT, qstr);
}

void Message::setOriginID(QString qstr)
{
    insert(KEY_ORIGINID, qstr);
}

void Message::setSeqNo(quint32 seqno)
{
    insert(KEY_SEQNO, seqno);
}

void Message::setWantMap(QVariantMap qvm)
{
    insert(KEY_WANT, qvm);
}

QString Message::getType()
{
    return value(KEY_TYPE).toString();
}

QString Message::getText()
{
    return value(KEY_CHATTEXT).toString();
}

QString Message::getOriginID()
{
    return value(KEY_ORIGINID).toString();
}

quint32 Message::getSeqNo()
{
    return value(KEY_SEQNO).toInt();
}

QVariantMap Message::getWantMap()
{
    return value(KEY_WANT).toMap();
}
