#include "Peer.hh"

Peer::Peer(QString qstr)
{
    QStringList args = qstr.split(":", QString::SkipEmptyParts);

    QHostAddress address;
    QList<QHostAddress> adresses;

    if(address.setAddress(args[0]))
    {
        adresses.append(address);
        setAddresses(&adresses);
        valid = true;
    }
    else
    {
        loookup(args[0], this, SLOT(hostResolved(QHostInfo)));
        valid = false;
    }

    port = args.at[1].toInt();
}

Peer::~Peer()
{}

quint32 Peer::getPort()
{
    return port;
}

QHostAddress Peer::getAddress()
{
    return adresses.at(0);
}

bool Peer::isValid()
{
    return valid;
}

QString Peer::toString()
{
    return addresses.at(0).toString() + ":" + QString::num(port);
}

bool operator==(Peer& p1, Peer& p2)
{
    return (p1->getAddress().toString() == p2->getAddress().toString()) &&
            (p1->getPort() == p2->getPort());
}

void Peer::hostResolved(QHostInfo info)
{
    setAddresses(info.addresses());
    setHostname(info.hostname());
    valid = true;
}

