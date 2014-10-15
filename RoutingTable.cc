#include "RoutingTable.hh"

RoutingTable::RoutingTable(Peerster* p)
    : peerster(p)
    , table(new QHash<QString,Peer>)
{}

RoutingTable::~RoutingTable()
{}

Peer Mailbox::nextHop(QString origin)
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

bool Mailbox::nextHopIsDirect(QString origin)
{
    if(messagestore->isNewOrigin(origin))
    {
        return false;
    }
    return table->value(origin).second == true;
}

void Mailbox::gotProcessRumorRoute(Message msg, Peer peer)
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
        if(messagestore->isNewOrigin(msgOrigin))
        {
            Q_EMIT(updateGUIOriginsList(msgOrigin));
            Q_EMIT(broadcastRoute());
        }
    }
    else if(messagestore->isNewOrigin(msgOrigin))
    {
        tableEntry.second = false;
        table->insert(msgOrigin, tableEntry);
        Q_EMIT(updateGUIOriginsList(msgOrigin));
        Q_EMIT(broadcastRoute());

    }
    else if(messagestore->isNewRumor(msg) && !nextHopIsDirect(msgOrigin))
    {
        tableEntry.second = false;
        table->insert(msgOrigin, tableEntry);
    }
}

void RoutingTable::gotSendDirect(Message msg,QString msgOrigin)
{
    Peer forwardPeer = routingTable->value(msgOrigin).first;
}

void Mailbox::gotBroadcastRoute()
{
    Message route = routeRumor();
    Q_EMIT(broadcast(route));
}




