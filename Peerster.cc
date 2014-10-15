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
    connect(mailbox, SIGNAL(displayMessage(Message)), 
        gui, SLOT(gotDisplayMessage(Message)));

    // outbound message signal chain
    connect(gui, SIGNAL(postToOutbox(Message)), 
        mailbox, SLOT(gotPostToOutbox(Message)));
    connect(mailbox, SIGNAL(sendMessage(Message,Peer)), 
        socket, SLOT(gotSendMessage(Message,Peer)));

    // message processing logic
    connect(messagestore, SIGNAL(canHelpPeer(Peer,QList<Message>)),
        mailbox, SLOT(gotCanHelpPeer(Peer,QList<Message>)));
    connect(messagestore, SIGNAL(needHelpFromPeer(Peer)),
        mailbox, SLOT(gotNeedHelpFromPeer(Peer)));
    connect(messagestore, SIGNAL(inConsensusWithPeer()),
        mailbox, SLOT(gotInConsensusWithPeer()));
    connect(messagestore, SIGNAL(updateGUIOriginsList(QString)),
        gui, SLOT(gotUpdateGUIOriginsList(QString)));
    connect(mailbox, SIGNAL(updateGUIOriginsList(QString)),
        gui, SLOT(gotUpdateGUIOriginsList(QString)));

    // add peers manually
    connect(gui, SIGNAL(potentialNewNeighbor(Peer)),
        mailbox, SLOT(gotPotentialNewNeighbor(Peer)));
    connect(gui, SIGNAL(sendStatusToPeer(Peer)),
        mailbox, SLOT(gotSendStatusToPeer(Peer)));
    connect(mailbox, SIGNAL(updateGUINeighbors(QList<Peer>)),
        gui, SLOT(gotUpdateGUINeighbors(QList<Peer>)));

    connect(messagestore, SIGNAL(broadcastRoute()),
        mailbox, SLOT(gotBroadcastRoute()));
    connect(table, SIGNAL(broadcast(Message)),
        this, SLOT(gotBroadcast(Message)));
    connect(table, SIGNAL(broadcastRoute()),
        this, SLOT(gotBroadcastRoute()));

    // DChat stuff
    connect(messagestore, SIGNAL(updateGUIDChatHistory(QString,QList<Message>)),
        gui, SLOT(gotUpdateGUIDChatHistory(QString,QList<Message>)));
    connect(gui, SIGNAL(getDChatHistoryFromOrigin(QString)),
        messagestore, SLOT(gotGetDChatHistoryFromOrigin(QString)));

    ///\\\
    connect(messagestore, SIGNAL(processRumorRoute(Message)),
        table, SLOT(gotProcessRumorRoute(Message)));
    connect(messagestore, SIGNAL(refreshGroupConvo()),
        gui, SLOT(gotRefreshGroupConvo()));
    connect(messagestore, SIGNAL(sendDirect(Message)),
        table, SLOT(gotSendDirect(Message)));
    connect(messagestore, SIGNAL(updateStatus(Message)),
        mailbox, SLOT(gotUpdateStatus(Message)));

    connect(messagestore, SIGNAL(monger(Message)),
        mailbox, SLOT(gotMonger(Message)));
    connect(messagestore, SIGNAL(broadcast(Message)),
        mailbox, SLOT(gotBroadcast(Message)));
    connect(messagestore, SIGNAL(needHelpFromPeer(Peer)),
        mailbox, SLOT(gotNeedHelpFromPeer(Peer)));
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

