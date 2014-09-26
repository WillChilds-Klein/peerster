#include "RoutingTable.hh"

RoutingTable::RoutingTable(Peerster* p)
    : peerster(p)
    , table(new QHash<QString,Peer>)
    , latest(new QHash<QString,quint32>)
{}

RoutingTable::~RoutingTable()
{}

QStringList RoutingTable::origins()
{
    return table->keys();
}

Peer RoutingTable::get(QString qstr)
{
    return table->value(qstr);
}

void RoutingTable::gotUpdateTable(Message msg, Peer peer)
{
    QString msgOrigin = msg.getOriginID();
    quint32 msgSeqno = msg.getSeqNo();

    if(isNewOrigin(msgOrigin))
    {
        Q_EMIT(monger(msg));
        Q_EMIT(updateGUIOriginsList(msgOrigin));
    }
    else if(msgSeqno >= latest->value(msgOrigin))
    {
        Q_EMIT(monger(msg));
    }

    if(isNewRumor(msg))
    {
        Q_EMIT(monger(msg));
        table->insert(msgOrigin, peer);
        latest->insert(msgOrigin, msgSeqno);
    }
}

bool RoutingTable::isNewRumor(Message msg)
{
    QString msgOrigin = msg.getOriginID();
    quint32 msgSeqno = msg.getSeqNo();

    return (isNewOrigin(msgOrigin) ||  msgSeqno > latest->value(msgOrigin));
}

bool RoutingTable::isNewOrigin(QString msgOrigin)
{
    return !origins().contains(msgOrigin);
}