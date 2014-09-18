#include "Peer.hh"

Peer::Peer(QString qstr)
    : addresses(new QList<QHostAddress>())
    , handler(new HostInfoHandler(this))
    , wellFormed(false)
{
    QString nameOrAddr;
    QHostAddress address;
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
                addresses->append(address);
            }
            else
            {
                qDebug() << "need to lookup hostname in DNS...";
                QHostInfo info = QHostInfo::fromName(nameOrAddr);
                addresses = new QList<QHostAddress>(info.addresses());
                valid = true;   
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
    if(!addresses->isEmpty()){
        return addresses->first();
    }
    qDebug() << "Oh Noes! address list is empty for" << port;
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
    addresses = new QList<QHostAddress>(newInfo.addresses());
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

