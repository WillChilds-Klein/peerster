#include "Socket.hh"

Socket::Socket(Peerster* p)
    : peerster(p)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(gotReadyRead()));
}

Socket::~Socket() 
{}

void Socket::setPortRange(quint32 min, quint32 max)
{
    myPortMin = min;
    myPortMax = max;
}

void Socket::setNoForward(bool b)
{
    noforward = b;
}

qint32 Socket::bind()
{
    for (quint32 p = myPortMin; p <= myPortMax; p++)
    {
        if (QUdpSocket::bind(p))
        {
            qDebug() << "bound to UDP port " << p;
            port = p;
            return p;
        }
    }

    qDebug() << "Oops, no ports in my default range " << myPortMin
        << "-" << myPortMax << " available";

    return -1;
}

void Socket::gotReadyRead()
{   
    QHostAddress senderAddr;
    QString senderInfo;
    quint16 senderPort;
    QByteArray datagram;
    quint64 size;

    while (hasPendingDatagrams()) 
    {
        datagram.resize(pendingDatagramSize());
        size = datagram.size();

        readDatagram(datagram.data(), size, &senderAddr, &senderPort);

        senderInfo = senderAddr.toString() + ":" 
                                           + QString::number(senderPort);
        Message msg = Message(&datagram);
        
        if(msg.isWellFormed()) 
        {
            Peer sender = Peer(senderInfo);

            qDebug() <<  "GOT MSG: " << msg.toString() 
                     << " FROM: " << sender.toString();

            Q_EMIT(processNeighbor(sender));
            Q_EMIT(postToInbox(msg, sender));
        }
        else
        {
            qDebug() << "GOT BAD MSG " << msg.toString() << " FROM: " << senderInfo;
        }
    }
}

void Socket::gotSendMessage(Message msg, Peer peer)
{
    QString type = msg.getType();
    if(!noforward || (type == TYPE_RUMOR_ROUTE))
    {
        QByteArray msgArr = msg.toSerializedQVMap();

        writeDatagram(msgArr, peer.getAddress(), peer.getPort());
    }
}


