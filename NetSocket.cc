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
        qDebug() << "Something recieved!" << senderInfo;
        Peer sender = Peer(senderInfo);
        if(sender.isValid())
        {
            Message msg = Message(&datagram);
            qDebug() << "NEW MSG RECIEVED OF SIZE" << msg.size() << ", KEYS:";
            foreach(QString qstr, msg.keys())
                qDebug() << qstr;
            qDebug() << "END KEYS";
            if(msg.isWellFormed()) 
            {
                Q_EMIT(potentialNewNeighbor(sender));
                Q_EMIT(postToInbox(msg, sender));

                qDebug() << "GOT GOOD MSG" << msg.toString() << "FROM: " << senderInfo;
            }
            else
            {
                qDebug() << "GOT BAD MSG FROM: " << senderInfo;
            }
        }
    }
}

void NetSocket::gotSendMessage(Message msg, Peer peer)
{
    if(peer.isValid())
    {
        // serialize map
        QByteArray msgArr = msg.toSerializedQVMap();

        qDebug() << "SEND MSG" << msg.toString() << "TO:" << peer.toString();

        // Send message via UDP
        writeDatagram(msgArr, peer.getAddress(), peer.getPort());
    }
}


