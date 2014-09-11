#include "MessageStore.hh"

MessageStore::MessageStore()
    : store(new QMap< QString, QList<Message> >())
    , latest(new QMap<QString, quint32>())
{}

MessageStore::~MessageStore()
{}

bool MessageStore::isNewRumor(Message msg)
{
    if(msg.typeIsStatus())
        return false;
    else if(!store->contains(msg.getOriginID()))
        return true;

    quint32 msgSeqNo = msg.getSeqNo();

    QList<Message> senderHistory = store->value(msg.getOriginID());

    if(!senderHistory.isEmpty() && 
        senderHistory.back().getSeqNo() > msgSeqNo)
        return false;
    else
        return true;
}

void MessageStore::addNewMessage(Message msg)
{
    QString origin = msg.getOriginID();
    QList<Message> originHistory;
    if(store->contains(origin))
    {
        originHistory = store->value(origin);
    }
    else
    {
        originHistory = QList<Message>();
    }

    store->insert(origin, originHistory);
}


QList<Message> MessageStore::getMessagesInRange(QString origin, 
    quint32 firstSeqNo, quint32 lastSeqNo)
{
    Message curr;
    quint32 currSeqNo;
    QList<Message> originHistory, ret;

    originHistory = store->value(origin);
    ret = QList<Message>();

    if(firstSeqNo > latest->value(origin))
        return ret;

    QListIterator<Message> i(originHistory);
    while (i.hasNext())
    {
        // inefficient access, consider assuming well-orderedness
        // and contiguity of SeqNo's in history and access element
        // index by seqNo - 1
        curr = i.next();
        currSeqNo = curr.getSeqNo();
        if(currSeqNo >= firstSeqNo && currSeqNo <= lastSeqNo)
            ret.append(curr);
    }

    return ret;
}

