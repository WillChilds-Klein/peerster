#include "Peer.hh"

Peer::Peer()
    : host(new QHostAddress(QHostAddress::LocalHost))
{}

Peer::Peer(quint32 p)
    : port(p)
    , host(new QHostAddress(QHostAddress::LocalHost)) 
{}

Peer::~Peer()
{}

quint32 Peer::getPort()
{
    return port;
}

QHostAddress Peer::getHost()
{
    return *host;
}