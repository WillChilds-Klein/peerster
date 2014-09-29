#include "RoutingTable.hh"

RoutingTable::RoutingTable(Peerster* p)
    : peerster(p)
    , table(new QHash<QString,Peer>)
    , latest(new QHash<QString,quint32>)
    , directness(new QHash<QString,bool>)
{}

RoutingTable::~RoutingTable()
{}