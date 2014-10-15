#include "RoutingTable.hh"

RoutingTable::RoutingTable(Peerster* p)
    : peerster(p)
    , table(new QHash<QString,Peer>())
    , invalid(new Peer("0.0.0.0:0"))
{}

RoutingTable::~RoutingTable()
{}

void RoutingTable::gotProcessRumorRoute(Message msg, Peer peer)
{
    QString msgOrigin = msg.getOriginID();
    QPair<Peer,bool> tableEntry;

    tableEntry.first = peer;

    if(msgOrigin == ID)
    {
        return;
    }
    else if(msg.isDirectRumor())
    {
        tableEntry.second = true;
        table->insert(msgOrigin, tableEntry);
    }
    else if(isNewOrigin(msgOrigin) || // <-- ? TODO
            (isNewRumor(msg) && !nextHopIsDirect(msgOrigin)))
    {
        tableEntry.second = false;
        table->insert(msgOrigin, tableEntry);
    }
}

void RoutingTable::gotSendDirect(Message msg,QString msgOrigin)
{
    Peer forwardPeer = routingTable->value(msgOrigin).first;
}

void RoutingTable::gotBroadcastRoute()
{
    Message route = routeRumor();
    Q_EMIT(broadcast(route));
}

Peer RoutingTable::nextHop(QString origin)
{
    if(table->keys().contains(origin))
    {
        return table->value(origin).first;
    }
    else
    {
        return *invalid;
    }
}

bool RoutingTable::nextHopIsDirect(QString origin)
{
    if(isNewOrigin(origin))
    {
        return false;
    }
    return (table->value(origin).second == true);
}

bool RoutingTable::isNewRumor(Message msg)
{
    if(!(msg.getType() == TYPE_RUMOR_CHAT || msg.getType() == TYPE_RUMOR_ROUTE))
    {
        return false;
    }
    else if(isNewOrigin(msg.getOriginID()))
    {
        return true;
    }

    quint32 msgSeqNo = msg.getSeqNo();
    QMap<QString, quint32> latest = latest();
    
    if(msgSeqNo > latest.value(msg.getOriginID()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool RoutingTable::isNewOrigin(QString origin)
{
    return !rumorStore->keys().contains(origin);
}

bool RoutingTable::isNextRumorInSeq(Message msg)
{
    QMap<QString, quint32> latest = latest();
    if(rumorStore->contains(msg.getOriginID()))
    {
        if(msg.getSeqNo() == latest.value(msg.getOriginID())+1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if(msg.getSeqNo() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

