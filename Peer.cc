#include "Peer.hh"

Peer::Peer(QString qstr)
    : info(new QHostInfo(0))
    , handler(new HostInfoHandler(this))
    , valid(false)
    , wellFormed(false)
{
    qDebug() << "IN PEER CONSTRUCTOR OF:" << qstr;

    QString nameOrAddr;
    QHostAddress address;
    QList<QHostAddress> addresses;
    bool isInt;

    QStringList args = qstr.split(":", QString::SkipEmptyParts);

    if(args.size() == 2)
    {    
        nameOrAddr = args.first(); 
        port = args.last().toInt(&isInt); 

        if(isInt)
        {
            wellFormed = true;

            if(address.setAddress(nameOrAddr))
            {
                qDebug() << "able to set address";
                addresses.append(address);
                info->setAddresses(addresses);
                valid = true;
            }
            else
            {
                qDebug() << "need to lookup hostname in DNS...";
                QHostInfo::lookupHost(nameOrAddr, handler, SIGNAL(hostResolved(QHostInfo)));
                valid = false; // until lookup completes.
            }
        }
    }
}

Peer::~Peer()
{}

quint32 Peer::getPort()
{
    return port;
}

QHostAddress Peer::getAddress()
{
    QList<QHostAddress> addrs = info->addresses();
    // qDebug() << addr.size() << "addresses for" << info->hostName();
    if(!addrs.isEmpty()){
        return addrs.first();
    }
 
    return QHostAddress::LocalHost;
}   

bool Peer::isValid()
{
    return valid;
}

bool Peer::isWellFormed()
{
    return wellFormed;
}

QString Peer::toString()
{
    return getAddress().toString() + ":" + QString::number(port);
}

void Peer::newHostInfo(QHostInfo newInfo)
{
    qDebug() << "Host info should be ready now!!";
	
    if(newInfo.error() != QHostInfo::NoError)
    {
        qDebug() << "BAD NEW HOST INFO!";
        return;
    }
    //*info = newInfo;
    //info.setAddresses(newInfo.addresses());
    //info.setHostName(newInfo.hostName());
    free(info);
    info = new QHostInfo(newInfo);
    valid = true;
}

bool Peer::operator==(Peer other)
{
    return (this->getAddress().toString() == other.getAddress().toString()) &&
            (this->getPort() == other.getPort());
} 

HostInfoHandler::HostInfoHandler(Peer* peer)
    : peer(peer)
{
    connect(this, SIGNAL(hostResolved(QHostInfo)),
        this, SLOT(gotHostResolved(QHostInfo)));
}

void HostInfoHandler::gotHostResolved(QHostInfo newInfo)
{
    peer->newHostInfo(newInfo);
}

