#include "RoutingTable.hh"

RoutingTable::RoutingTable(Peerster* p)
    : peerster(p)
    , table(new QHash<QString,Peer>)
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
    table->insert(msgOrigin, peer);
}