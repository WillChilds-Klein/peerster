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
        else if(contains(KEY_ORIGINID) && contains(KEY_SEQNO) &&
                contains(KEY_CHATTEXT))
        {
            setType(TYPE_RUMOR_CHAT);
        }
        else if(contains(KEY_ORIGINID) && contains(KEY_SEQNO) &&
               !contains(KEY_CHATTEXT))
        {
            setType(TYPE_RUMOR_ROUTE);
        }
        else if(contains(KEY_ORIGINID) && contains(KEY_CHATTEXT) && 
                contains(KEY_HOPLIMIT) && contains(KEY_DEST))
        {
            setType(TYPE_DIRECT_CHAT);
        }
        else if(contains(KEY_BLOCKREQUEST))
        {
            setType(TYPE_BLOCK_REQUEST);
        }
        else if(contains(KEY_BLOCKREPLY))
        {
            setType(TYPE_BLOCK_REPLY);
        }
        else if(contains(KEY_SEARCH))
        {
            setType(TYPE_SEARCH_REQUEST);
        }
        else if(contains(KEY_SEARCHREPLY))
        {
            setType(TYPE_SEARCH_REPLY);
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

    if(getType() == TYPE_STATUS) 
    {
        str += "[Want: ";
        QVariantMap wantMap = value(KEY_WANT).toMap();
        for(i = wantMap.begin(); i != wantMap.end(); ++i)
        {
            str += "<" + i.key() + ": " + i.value().toString() + ">,";
        }
        str += "]";
    }
    else if(getType() == TYPE_BLOCK_REPLY) 
    {
        str += "[";
        for(i = this->begin(); i != this->end(); ++i)
        {
            if(i.key() == KEY_BLOCKREPLY || i.key() == KEY_DATA)
            {
                str += "<" + i.key() + ": " + 
                             QString(i.value().toByteArray().toHex()) + ">,";
            }
            else
            {
                str += "<" + i.key() + ": " + i.value().toString() + ">,";
            }
        }
        str += "]";
    }
    else if(getType() == TYPE_BLOCK_REQUEST) 
    {
        str += "[";
        for(i = this->begin(); i != this->end(); ++i)
        {
            if(i.key() == KEY_BLOCKREQUEST)
            {
                str += "<" + i.key() + ": " + 
                             QString(i.value().toByteArray().toHex()) + ">,";
            }
            else
            {
                str += "<" + i.key() + ": " + i.value().toString() + ">,";
            }
        }
        str += "]";
    }
    else if(getType() == TYPE_SEARCH_REPLY)
    {
        str += "[";
        for(i = this->begin(); i != this->end(); ++i)
        {
            if(i.key() == KEY_MATCHNAMES)
            {
                QVariantList matchNames = i.value().toList();
                str += "<" + i.key() + ": [";
                QVariantList::iterator j;
                for(j = matchNames.begin(); j != matchNames.end(); ++j)
                {
                    str += j->toString() + ",";
                }
                str += "]>,";
            }
            else if(i.key() == KEY_MATCHIDS)
            {
                QByteArray matches = i.value().toByteArray();
                str += "<" + i.key() + ": ["; 
                for(int j = 0; j < matches.size(); j += HASH_SIZE)
                {
                    str += QString(i.value().toByteArray().mid(j, HASH_SIZE).toHex())+",";
                }
                str += "]>,";
            }
            else
            {
                str += "<" + i.key() + ": " + i.value().toString() + ">,";
            }
        }
        str += "]";
    }
    else
    {
        str += "[";
        for(i = this->begin(); i != this->end(); ++i)
        {
            str += "<" + i.key() + ": " + i.value().toString() + ">,";
        }
        str += "]";
    }

    return str;
}

QByteArray Message::toSerializedQVMap()
{
    QByteArray msgArr;
    QDataStream stream(&msgArr, QIODevice::WriteOnly);

    QVariantMap map;
    if(getType() == TYPE_RUMOR_CHAT || getType() == TYPE_RUMOR_ROUTE)
    {
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_SEQNO, value(KEY_SEQNO));
        if(getType() == TYPE_RUMOR_CHAT)
        {
            map.insert(KEY_CHATTEXT, value(KEY_CHATTEXT));
        }
        if(contains(KEY_LASTIP) && contains(KEY_LASTPORT))
        {
            map.insert(KEY_LASTIP, value(KEY_LASTIP));
            map.insert(KEY_LASTPORT, value(KEY_LASTPORT));
        }
    }
    else if(getType() == TYPE_STATUS)
    {
        map.insert(KEY_WANT, value(KEY_WANT));
    }
    else if(getType() == TYPE_DIRECT_CHAT)
    {
        map.insert(KEY_CHATTEXT, value(KEY_CHATTEXT));
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_DEST, value(KEY_DEST));
        map.insert(KEY_HOPLIMIT, value(KEY_HOPLIMIT));
    }
    else if(getType() == TYPE_BLOCK_REQUEST)
    {
        map.insert(KEY_DEST, value(KEY_DEST));
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_HOPLIMIT, value(KEY_HOPLIMIT));
        map.insert(KEY_BLOCKREQUEST, value(KEY_BLOCKREQUEST));
    }
    else if(getType() == TYPE_BLOCK_REPLY)
    {
        map.insert(KEY_DEST, value(KEY_DEST));
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_HOPLIMIT, value(KEY_HOPLIMIT));
        map.insert(KEY_BLOCKREPLY, value(KEY_BLOCKREPLY));
        map.insert(KEY_DATA, value(KEY_DATA));
    }
    else if(getType() == TYPE_SEARCH_REQUEST)
    {
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_SEARCH, value(KEY_BUDGET));
        map.insert(KEY_BUDGET, value(KEY_BUDGET));
    }
    else if(getType() == TYPE_SEARCH_REPLY)
    {
        map.insert(KEY_DEST, value(KEY_DEST));
        map.insert(KEY_ORIGINID, value(KEY_ORIGINID));
        map.insert(KEY_HOPLIMIT, value(KEY_HOPLIMIT));
        map.insert(KEY_SEARCHREPLY, value(KEY_SEARCHREPLY));
        map.insert(KEY_MATCHNAMES, value(KEY_MATCHNAMES));
        map.insert(KEY_MATCHIDS, value(KEY_MATCHIDS));
    }

    stream << map;

    return msgArr;
}

bool Message::isWellFormed()
{
    bool isStatus, isRumor, isDChat, isBlock, isSearch;
    isStatus = contains(KEY_WANT);
    isRumor = (contains(KEY_ORIGINID) && contains(KEY_SEQNO));
    isDChat = (contains(KEY_ORIGINID) && contains(KEY_CHATTEXT) && 
               contains(KEY_HOPLIMIT) && contains(KEY_DEST));
    isBlock = (contains(KEY_DEST) && contains(KEY_ORIGINID) &&
               contains(KEY_HOPLIMIT) && 
              (contains(KEY_BLOCKREQUEST) || 
              (contains(KEY_BLOCKREPLY) && contains(KEY_DATA))));
    isSearch = ((contains(KEY_SEARCH) && contains(KEY_ORIGINID) && 
                 contains(KEY_BUDGET)) || 
                (contains(KEY_SEARCHREPLY) && contains(KEY_DEST) && 
                 contains(KEY_HOPLIMIT) && contains(KEY_SEARCHREPLY) &&
                 contains(KEY_MATCHNAMES) && contains(KEY_MATCHIDS)));
    
    if(getType() == TYPE_BLOCK_REPLY && !isValidBlockReply())
    {
        qDebug() << "INVALID BLOCK REPLY!!";
        return false;
    }
    else if(isStatus || isRumor || isDChat || isBlock || isSearch)
    {
        return true;
    }

    return false;
}

bool Message::isEmptyStatus()
{
    return value(KEY_WANT).toMap().size() == 0;
}

bool Message::isDirectRumor()
{
    return (getType() == TYPE_RUMOR_CHAT || getType() == TYPE_RUMOR_ROUTE) && 
           (!contains(KEY_LASTIP) && !contains(KEY_LASTPORT));
}

bool Message::isValidBlockReply()
{
    if(getType() != TYPE_BLOCK_REPLY)
    {
        return false;
    }

    QCA::Hash sha("sha1");
    sha.update(getData()); 
    QByteArray hash = sha.final().toByteArray();

    qDebug() << "ATTN: " << QString(hash.toHex()) << "=?=" 
             << QString(getBlockReply().toHex());
    qDebug() << "DATA: " << QString(getData().toHex());

    return hash == getBlockReply();
}

void Message::setType(QString str)
{
    insert(KEY_TYPE, str);
}

void Message::setChatText(QString qstr)
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

void Message::setDest(QString qstr)
{
    insert(KEY_DEST, qstr);
}

void Message::setHopLimit(quint32 lim)
{
    insert(KEY_HOPLIMIT, lim);
}

void Message::setLastIP(quint32 IP)
{
    insert(KEY_LASTIP, IP);
}

void Message::setLastPort(quint16 port)
{
    insert(KEY_LASTPORT, port);
}

void Message::setBlockRequest(QByteArray arr)
{
    insert(KEY_BLOCKREQUEST, arr);
}

void Message::setBlockReply(QByteArray arr)
{
    insert(KEY_BLOCKREPLY, arr);
}

void Message::setData(QByteArray arr)
{
    insert(KEY_DATA, arr);
}

void Message::setSearch(QString qstr)
{
    insert(KEY_SEARCH, qstr);
}

void Message::setBudget(quint32 bud)
{
    insert(KEY_BUDGET, bud);
}

void Message::setSearchReply(QString qstr)
{
    insert(KEY_SEARCHREPLY, qstr);
}

void Message::setMatchNames(QVariantList qvl)
{
    insert(KEY_MATCHNAMES, qvl);
}

void Message::setMatchIDs(QByteArray arr)
{
    insert(KEY_MATCHIDS, arr);
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

QString Message::getDest()
{
    return value(KEY_DEST).toString();
}

quint32 Message::getHopLimit()
{
    return value(KEY_HOPLIMIT).toInt();
}

quint32 Message::getLastIP()
{
    return value(KEY_LASTIP).toInt();
}

quint16 Message::getLastPort()
{
    return value(KEY_LASTPORT).toInt();
}

QByteArray Message::getBlockRequest()
{
    return value(KEY_BLOCKREQUEST).toByteArray();
}

QByteArray Message::getBlockReply()
{
    return value(KEY_BLOCKREPLY).toByteArray();
}

QByteArray Message::getData()
{
    return value(KEY_DATA).toByteArray();
}

QString Message::getSearch()
{
    return value(KEY_SEARCH).toString();
}

quint32 Message::getBudget()
{
    return value(KEY_BUDGET).toInt();
}

QString Message::getSearchReply()
{
    return value(KEY_SEARCHREPLY).toString();
}

QVariantList Message::getMatchNames()
{
    return value(KEY_MATCHNAMES).toList();
}

QByteArray Message::getMatchIDs()
{
    return value(KEY_MATCHIDS).toByteArray();
}

