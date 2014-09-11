#include "NetSocket.hh"

NetSocket::NetSocket(Peerster* p)
    : peerster(p)
{
    // Pick a range of four UDP ports to try to allocate by default,
    // computed based on my Unix user ID.
    // This makes it trivial for up to four Peerster instances per user
    // to find each other on the same host,
    // barring UDP port conflicts with other applications
    // (which are quite possible).
    // We use the range from 32768 to 49151 for this purpose.
    myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;
    port = -1;

    connect(this, SIGNAL(readyRead()), this, SLOT(gotReadyRead()));

    // Create a UDP network socket
    if (!bind())
    {
        qDebug() << "Peerster failed to bind!";
        exit(1);
    }
    p->setPort(port);
    neighbors = findNeighbors();

    // print neighbors.
    qDebug() << "Neighbors: ";
    quint32 i; foreach(i, neighbors)
        qDebug() << i << " ";
}

NetSocket::~NetSocket() 
{}

int NetSocket::getPort()
{
    return port;
}

bool NetSocket::bind()
{
    // Try to bind to each of the range myPortMin..myPortMax in turn.
    for (quint32 p = myPortMin; p <= myPortMax; p++)
    {
        if (QUdpSocket::bind(p))
        {
            qDebug() << "bound to UDP port " << p;
            port = p;
            return true;
        }
    }

    qDebug() << "Oops, no ports in my default range " << myPortMin
        << "-" << myPortMax << " available";

    return false;
}

void NetSocket::send(Message msg){
    // serialize map
    QByteArray msgArr = msg.serialize();

    // save host address
    QHostAddress host = QHostAddress(QHostAddress::LocalHost);

    // Send message via UDP
    for(quint32 p = myPortMin; p <= myPortMax; p++)
    {
        if(p != this->port)
        {
            writeDatagram(msgArr, host, p);
        }
    }

    qDebug() << "SENT:" << msg.toString();
}

QList<quint32> NetSocket::findNeighbors()
{
    QList<quint32> neighbors;

    if(port != myPortMin)
        neighbors.append(port-1);
    if(port != myPortMax)
        neighbors.append(port+1);

    return neighbors;
}

void NetSocket::pickNewPeer()
{
    peerPort = neighbors.at(qrand() % 2);
}

void NetSocket::gotReadyRead()
{   
    QHostAddress senderAddr;
    quint16 senderPort;
    QByteArray datagram;
    quint64 size;
    while (hasPendingDatagrams()) 
    {
        datagram.resize(pendingDatagramSize());
        size = datagram.size();

        readDatagram(datagram.data(), size, &senderAddr, &senderPort);

        if(!peered || senderPort == peerPort)
        {
            Message msg = Message(&datagram);
            msg.setPortOfOrigin(port);    
            Q_EMIT(postToInbox(msg));
        }
    }
}

void NetSocket::gotSendMessage(Message msg)
{
    send(msg);
}

void NetSocket::gotStartPeering(quint32 p)
{
    peerPort = p;
    peered = true;
}

void NetSocket::gotStopPeering()
{
    qDebug() << "Peering with " + QString::number(peerPort) + " stopped!";
    peerPort = -1;
    peered = false;
}


