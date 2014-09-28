#include "RoutingTable.hh"

RoutingTable::RoutingTable(Peerster* p)
    : peerster(p)
    , table(new QHash<QString,Peer>)
    , latest(new QHash<QString,quint32>)
    , directness(new QHash<QString,bool>)
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
        Q_EMIT(broadcastRoute());
        Q_EMIT(updateGUIOriginsList(msgOrigin));
    }

    if(msg.isDirectRumor())
    {
        table->insert(msgOrigin, peer);
        directness->insert(msgOrigin, true);
    }

    if(isNewRumor(msg))
    {
        Q_EMIT(monger(msg));
        latest->insert(msgOrigin, msgSeqno);
        if(!directness->contains(msgOrigin) || directness->value(msgOrigin) == false)
        {
            table->insert(msgOrigin, peer);
            if(!msg.isDirectRumor())
            {
                directness->insert(msgOrigin, false);
            }
        }
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