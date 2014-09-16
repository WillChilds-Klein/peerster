#include "NetSocket.hh"

NetSocket::NetSocket(Peerster* p)
    : peerster(p)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(gotReadyRead()));
}

NetSocket::~NetSocket() 
{}

quint32 NetSocket::getPort()
{
    return port;
}

void NetSocket::setPortRange(quint32 min, quint32 max)
{
    myPortMin = min;
    myPortMax = max;
}

qint32 NetSocket::bind()
{
    // Try to bind to each of the range myPortMin..myPortMax in turn.
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

        Message msg = Message(&datagram); 
        Q_EMIT(postToInbox(msg));

        qDebug() << "RECEIVED FROM PORT " << senderPort 
            << ": " << msg.toString() << "type: " << msg.getType();
    }
}

void NetSocket::gotSendMessage(Message msg, Peer peer)
{
    // serialize map
    QByteArray msgArr = msg.serialize();

    // Send message via UDP
    writeDatagram(msgArr, peer.getHost(), peer.getPort());

    qDebug() << "SENT: " << msg.toString() << "TO PORT: " << peer.getPort();
}


