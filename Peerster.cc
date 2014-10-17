#include "Peerster.hh"

Peerster::Peerster()
    : gui(new GUI(this))
    , socket(new Socket(this))
    , mailbox(new Mailbox(this))
    , messagestore(new MessageStore(this))
    // , filestore(new FileStore(this))
{
    // GUI
    connect(gui, SIGNAL(processNeighbor(Peer)),
        mailbox, SLOT(gotProcessNeighbor(Peer)));
    connect(gui, SIGNAL(createChatRumor(QString)), 
        messagestore, SLOT(gotCreateChatRumor(QString)));
    connect(gui, SIGNAL(createDirectChat(QString,QString)),
        messagestore, SLOT(gotCreateDirectChat(QString,QString)));
    // connect(gui, SIGNAL(processShareFiles(QStringList)),
    //     filestore, SLOT(gotProcessShareFiles(QStringList)));

    // Socket
    connect(socket, SIGNAL(postToInbox(Message,Peer)), 
        mailbox, SLOT(gotPostToInbox(Message,Peer)));
    connect(socket, SIGNAL(processNeighbor(Peer)),
        mailbox, SLOT(gotProcessNeighbor(Peer)));

    // Mailbox
    connect(mailbox, SIGNAL(sendMessage(Message,Peer)), 
        socket, SLOT(gotSendMessage(Message,Peer)));
    connect(mailbox, SIGNAL(refreshNeighbors(QList<Peer>)),
        gui, SLOT(gotRefreshNeighbors(QList<Peer>)));
    connect(mailbox, SIGNAL(processRumor(Message,Peer)),
        messagestore, SLOT(gotProcessRumor(Message,Peer)));
    connect(mailbox, SIGNAL(processRumorRoute(Message,Peer)),
        messagestore, SLOT(gotProcessRumorRoute(Message,Peer)));
    connect(mailbox, SIGNAL(processDirectChat(Message)),
        messagestore, SLOT(gotProcessDirectChat(Message)));
    connect(mailbox, SIGNAL(processIncomingStatus(Message,Peer)),
        messagestore, SLOT(gotProcessIncomingStatus(Message,Peer)));
    connect(mailbox, SIGNAL(broadcastRoute()), 
        messagestore, SLOT(gotBroadcastRoute()));

    // MessageStore
    connect(messagestore, SIGNAL(helpPeer(Peer,QList<Message>)),
        mailbox, SLOT(gotHelpPeer(Peer,QList<Message>)));
    connect(messagestore, SIGNAL(needHelpFromPeer(Peer)),
        mailbox, SLOT(gotNeedHelpFromPeer(Peer)));
    connect(messagestore, SIGNAL(inConsensusWithPeer()),
        mailbox, SLOT(gotInConsensusWithPeer()));
    connect(messagestore, SIGNAL(processNeighbor(Peer)),
        mailbox, SLOT(gotProcessNeighbor(Peer)));
    connect(messagestore, SIGNAL(monger(Message)),
        mailbox, SLOT(gotMonger(Message)));
    connect(messagestore, SIGNAL(updateStatus(Message)),
        mailbox, SLOT(gotUpdateStatus(Message)));
    connect(messagestore, SIGNAL(processRumorRoute(Message,Peer)),
        messagestore, SLOT(gotProcessRumorRoute(Message,Peer)));
    connect(messagestore, SIGNAL(refreshGroupConvo()),
        gui, SLOT(gotRefreshGroupConvo()));
    connect(messagestore, SIGNAL(refreshDirectConvo(QString)),
        gui, SLOT(gotRefreshDirectConvo(QString)));
    connect(messagestore, SIGNAL(refreshOrigins(QStringList)),
        gui, SLOT(gotRefreshOrigins(QStringList)));
    connect(messagestore, SIGNAL(sendMessage(Message,Peer)),
        socket, SLOT(gotSendMessage(Message,Peer)));
    connect(messagestore, SIGNAL(broadcast(Message)),
        mailbox, SLOT(gotBroadcast(Message)));


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

    QMap<QString, quint32>* sharedFileInfo = new QMap<QString, quint32>;
    QList<Message>* groupConvo = new QList<Message>;
    QMap< QString,QList<Message> >* directStore = 
        new QMap< QString,QList<Message> >;

    Peer* self = new Peer("127.0.0.1:" + QString::number(port));

    gui->setWindowTitle("Peerster Instance " + ID + " on port " + QString::number(port));
    gui->setID(ID);
    gui->setSharedFileInfo(sharedFileInfo);
    gui->setGroupConvo(groupConvo);
    gui->setDirectStore(directStore);

    mailbox->setPortInfo(myPortMin, myPortMax, port);
    mailbox->setSelfPeer(self);

    messagestore->setID(ID);
    messagestore->setSelfPeer(self);
    messagestore->setGroupConvo(groupConvo);
    messagestore->setDirectStore(directStore);

    // filestore->setSharedFileInfo(sharedfileinfo);
    // filestore->setID(ID);

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

    mailbox->populateNeighbors();
}

Peerster::~Peerster()
{}

void Peerster::run()
{   
    // Create an initial chat gui window
    gui->show();
}

