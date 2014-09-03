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
}

NetSocket::~NetSocket() {}

bool NetSocket::bind()
{
    // Try to bind to each of the range myPortMin..myPortMax in turn.
    for (int p = myPortMin; p <= myPortMax; p++) {
        if (QUdpSocket::bind(p)) {
            qDebug() << "bound to UDP port " << p;
            return true;
        }
    }

    qDebug() << "Oops, no ports in my default range " << myPortMin
        << "-" << myPortMax << " available";
    return false;
}

void NetSocket::send(Message msg){
    // qDebug() << "NetSocket::send invoked";

    // Serialize map
    QByteArray messageArr;
    QDataStream stream(&messageArr, QIODevice::WriteOnly);
    stream << msg;

    /**/ // Deserializing stuff for testing
    QMap<QString, QVariant> dMap;
    QDataStream dStream(&messageArr, QIODevice::ReadOnly);
    dStream >> dMap;
    /**/

    // Send message via UDP
    for(int port = myPortMin; port <= myPortMax; port++){
        writeDatagram(messageArr, QHostAddress(QHostAddress::LocalHost), port);
        
        qDebug() << "MESSAGE: " << dMap.value("ChatText") << "\n SENT TO PORT: " << port;
    }
}