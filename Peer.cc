#include "Peer.hh"

Peer::Peer()
    : info(new QHostInfo(0))
    // , handler(new HostInfoHandler(this))
{}

Peer::Peer(QString qstr)
    : info(new QHostInfo(0))
    // , handler(new HostInfoHandler(this))
{
    QStringList args = qstr.split(":", QString::SkipEmptyParts);

    qDebug() << "IN PEER CONSTRUCTOR:" << qstr;

    QHostAddress address;
    QList<QHostAddress> addresses;

    if(address.setAddress(args.at(0)))
    {
        // qDebug() << "able to set address";
        addresses.append(address);
        info->setAddresses(addresses);
        // valid = true;
    }
    else
    {
        // QHostInfo::lookupHost(args[0], handler, SIGNAL(hostResolved(QHostInfo)));
        // valid = false;
        *info = QHostInfo::fromName(args.at(0));
    }

    port = args.at(1).toInt();
}

Peer::~Peer()
{}

quint32 Peer::getPort()
{
    return port;
}

QHostAddress Peer::getAddress()
{
    QList<QHostAddress> addr = info->addresses();
    qDebug() << addr.size() << "addresses for" << info->hostName();
    return addr.at(0);
}

// bool Peer::isValid()
// {
//     return valid;
// }

QString Peer::toString()
{
    return info->addresses().at(0).toString() + ":" + QString::number(port);
}

// void Peer::newHostInfo(QHostInfo newInfo)
// {
//     qDebug() << "yoyoyoyoyo";
//     if(newInfo.error() != QHostInfo::NoError)
//     {
//         qDebug() << "bad hostinfo!";
//         return;
//     }
//     qDebug() << "ayayaya" << newInfo.addresses().at(0).toString();
//     *info = newInfo;
//     valid = true;
// }

bool Peer::operator==(Peer other)
{
    return (this->getAddress().toString() == other.getAddress().toString()) &&
            (this->getPort() == other.getPort());
} 

// HostInfoHandler::HostInfoHandler(Peer* p)
//     : peer(p)
// {
//     connect(this, SIGNAL(hostResolved(QHostInfo)),
//         this, SLOT(gotHostResolved(QHostInfo)));
// }

// void HostInfoHandler::gotHostResolved(QHostInfo newInfo)
// {
//     peer->newHostInfo(newInfo);
// }

