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
    for (int p = myPortMin; p <= myPortMax; p++) {
        if (QUdpSocket::bind(p)) {
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
    for(int p = myPortMin; p <= myPortMax; p++){
        if(p != this->port)
        {
            writeDatagram(msgArr, host, p);
        }
    }
}

void NetSocket::gotReadyRead()
{   
    QHostAddress sender = QHostAddress::LocalHost;
    QByteArray datagram;
    quint16 pMin = (quint16) myPortMin;
    quint64 size;
    while (hasPendingDatagrams()) 
    {
        datagram.resize(pendingDatagramSize());
        size = datagram.size();

        readDatagram(datagram.data(), size, &sender, &pMin);

        Message msg = Message(&datagram);
        Q_EMIT(postToInbox(msg));
    }
}

void NetSocket::gotSendMessage(Message msg)
{
    send(msg);
}

