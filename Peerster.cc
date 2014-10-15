#include "Peerster.hh"

Peerster::Peerster()
    : gui(new GUI(this))
    , socket(new Socket(this))
    , table(new RoutingTable(this))
    , mailbox(new Mailbox(this))
    , messagestore(new MessageStore(this))
{
    // inbound message signal chain
    connect(socket, SIGNAL(postToInbox(Message,Peer)), 
        mailbox, SLOT(gotPostToInbox(Message,Peer)));

    // outbound message signal chain
    connect(gui, SIGNAL(postToOutbox(Message)), 
        mailbox, SLOT(gotPostToOutbox(Message)));
    connect(mailbox, SIGNAL(sendMessage(Message,Peer)), 
        socket, SLOT(gotSendMessage(Message,Peer)));

    // message processing logic
    connect(messagestore, SIGNAL(helpPeer(Peer,QList<Message>)),
        mailbox, SLOT(gotHelpPeer(Peer,QList<Message>)));
    connect(messagestore, SIGNAL(needHelpFromPeer(Peer)),
        mailbox, SLOT(gotNeedHelpFromPeer(Peer)));
    connect(messagestore, SIGNAL(inConsensusWithPeer()),
        mailbox, SLOT(gotInConsensusWithPeer()));

    // GUI
    connect(messagestore, SIGNAL(refreshGroupConvo()),
        gui, SLOT(gotRefreshGroupConvo()));
    connect(messagestore, SIGNAL(refreshDirectConvo(QString)),
        gui, SLOT(gotRefreshDirectConvo(QString)));
    connect(messagestore, SIGNAL(refreshOrigins(QString)),
        gui, SLOT(gotRefreshOrigins(QString)));
    connect(mailbox, SIGNAL(refreshNeighbors(QList<Peer>)),
        gui, SLOT(gotRefreshNeighbors(QList<Peer>)));

    // adding neighbors
    connect(gui, SIGNAL(processNeighbor(Peer)),
        mailbox, SLOT(processNeighbor(Peer)));

    // mongering & broadcasting
    connect(mailbox, SIGNAL(broadcastRoute()),
        messagestore, SLOT(gotBroadcastRoute()));
    connect(messagestore, SIGNAL(broadcast(Message)),
        mailbox, SLOT(gotBroadcast(Message)));
    connect(messagestore, SIGNAL(monger(Message)),
        mailbox, SLOT(gotMonger(Message)));

    ///\\\ new
    connect(messagestore, SIGNAL(sendDirect(Message)),
        table, SLOT(gotSendDirect(Message)));
    // connect(messagestore, SIGNAL(sendMessage(Message,Peer)),
    //     socket, SLOT(gotSendMessage(Message,Peer)));
    connect(messagestore, SIGNAL(processRumorRoute(Message)),
        table, SLOT(gotProcessRumorRoute(Message)));

    connect(messagestore, SIGNAL(updateStatus(Message)),
        mailbox, SLOT(gotUpdateStatus(Message)));
    ///\\\

    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    ID = QString::number((qrand() % ID_MAX) + 1);
    qDebug() << "Instance ID: "<< ID; 

    myPortMin = 32718 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 3;

    socket->setPortRange(myPortMin, myPortMax);
    port = socket->bind();
    if(port < 0)
    {
        qDebug() << "Peerster failed to bind to a port!";
        exit(1);
    }

    QMap< QString, QList<Message> >* directStore = 
        new QMap< QString, QList<Message> >();
    QList<Message>* groupConvo = new QList<Message>();

    QString title = "Peerster Instance " + ID + " on port " + QString::number(port);
    gui->setWindowTitle(title);
    gui->setGroupConvo(groupConvo);
    gui->setDirectStore(directStore);

    messagestore->setID(ID);
    messagestore->setGroupConvo(groupConvo);
    messagestore->setDirectStore(directStore);

    table->setMessageStore(messagestore);

    mailbox->setPortInfo(myPortMin, myPortMax, port);
    mailbox->setID(ID);
    mailbox->populateNeighbors();

    // noforward stuff
    noforward = false;
    QStringList clargs = QCoreApplication::arguments();
    for(int i = 1; i < clargs.size(); i++)
    {
        if(clargs.at(i) == SWITCH_NOFORWARD)
        {
            noforward = true;
        }
    }
    socket->setNoForward(noforward);
}

Peerster::~Peerster()
{}

void Peerster::run()
{   
    // Create an initial chat gui window
    gui->show();
}

