#include "Peer.hh"

Peer::Peer()
{}

Peer::Peer(QString qstr)
    : addresses(new QList<QHostAddress>())
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
                addresses->append(address);
            }
            else
            {
                // qDebug() << "need to lookup hostname in DNS...";
                QHostInfo info = QHostInfo::fromName(nameOrAddr);
                
                if(info.error() == QHostInfo::NoError)
                {
                    addresses = new QList<QHostAddress>(info.addresses());
                }
                else
                {
                    qDebug() << "Couldn't resolve DNS hostname!";
                }
            }
        }
    }
}

Peer::Peer(quint32 IPv4, quint16 p)
    : addresses(new QList<QHostAddress>())
    , wellFormed(true)
{
    addresses->append(QHostAddress(IPv4));
    port = p;
}

Peer::~Peer()
{}

quint16 Peer::getPort()
{
    return port;
}

QHostAddress Peer::getAddress()
{
    if(!addresses->isEmpty()){
        return addresses->first();
    }

    // qDebug() << "Oh Noes! address list is empty for this Peer!";
    return QHostAddress::LocalHost;
}

bool Peer::isWellFormed()
{
    return wellFormed;
}

QString Peer::toString()
{
    return (addresses->isEmpty() ? "NONE" : addresses->first().toString()) + ":" + QString::number(port);
    // return getAddress().toString() + ":" + QString::number(port);
}

bool Peer::operator==(Peer other)
{
    return (this->getAddress().toString() == other.getAddress().toString()) &&
            (this->getPort() == other.getPort());
} 

bool Peer::operator!=(Peer other)
{
    return !(*this == other);
}
