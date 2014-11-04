#include "Peerster.hh"

Peerster::Peerster()
    : gui(new GUI(this))
    , socket(new Socket(this))
    , mailbox(new Mailbox(this))
    , messagestore(new MessageStore(this))
    , filestore(new FileStore(this))
{
    // TODO: make all components take InstanceID as constructor paramerter.

    // GUI
    connect(gui, SIGNAL(processNeighbor(Peer)),
        mailbox, SLOT(gotProcessNeighbor(Peer)));
    connect(gui, SIGNAL(createChatRumor(QString)), 
        messagestore, SLOT(gotCreateChatRumor(QString)));
    connect(gui, SIGNAL(createDirectChat(QString,QString)),
        messagestore, SLOT(gotCreateDirectChat(QString,QString)));
    connect(gui, SIGNAL(processFilesToShare(QStringList)),
        filestore, SLOT(gotProcessFilesToShare(QStringList)));
    connect(gui, SIGNAL(searchForKeywords(QString)),
        filestore, SLOT(gotSearchForKeywords(QString)));
    connect(gui, SIGNAL(requestFileFromPeer(QString,QPair<QString,QByteArray>)),
        filestore, SLOT(gotRequestFileFromPeer(QString,QPair<QString,QByteArray>)));

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
    connect(mailbox, SIGNAL(processBlockRequest(Message)),
        filestore, SLOT(gotProcessBlockRequest(Message)));
    connect(mailbox, SIGNAL(processBlockReply(Message)),
        filestore, SLOT(gotProcessBlockReply(Message)));
    connect(mailbox, SIGNAL(processSearchRequest(Message)),
        filestore, SLOT(gotProcessSearchRequest(Message)));
    connect(mailbox, SIGNAL(processSearchReply(Message)),
        filestore, SLOT(gotProcessSearchReply(Message)));
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

    // FileStore
    connect(filestore, SIGNAL(refreshSharedFiles()),
        gui, SLOT(gotRefreshSharedFiles()));
    connect(filestore, SIGNAL(refreshDownloadInfo()),
        gui, SLOT(gotRefreshDownloadInfo()));
    connect(filestore, SIGNAL(sendDirect(Message,QString)),
        messagestore, SLOT(gotSendDirect(Message,QString)));
    connect(filestore, SIGNAL(postToInbox(Message,Peer)),
        mailbox, SLOT(gotPostToInbox(Message,Peer)));
    connect(filestore, SIGNAL(refreshSearchResults()),
        gui, SLOT(gotRefreshSearchResults()));


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
    QMap<QString,DownloadStatus::Status>* 
        downloadInfo = new QMap<QString,DownloadStatus::Status>;
    QMultiHash<QString, QPair<QString,QByteArray> >* 
        searchResults = new QMultiHash<QString, QPair<QString,QByteArray> >;
    QList<Message>* groupConvo = new QList<Message>;
    QMap< QString,QList<Message> >* directStore = 
        new QMap< QString,QList<Message> >;

    Peer* self = new Peer("127.0.0.1:" + QString::number(port));

    gui->setWindowTitle("Peerster Instance " + ID + " on port " + QString::number(port));
    gui->setID(ID);
    gui->setSharedFileInfo(sharedFileInfo);
    gui->setDownloadInfo(downloadInfo);
    gui->setSearchResults(searchResults);
    gui->setGroupConvo(groupConvo);
    gui->setDirectStore(directStore);

    mailbox->setPortInfo(myPortMin, myPortMax, port);
    mailbox->setSelfPeer(self);

    messagestore->setID(ID);
    messagestore->setSelfPeer(self);
    messagestore->setGroupConvo(groupConvo);
    messagestore->setDirectStore(directStore);

    filestore->setID(ID);
    filestore->setSharedFileInfo(sharedFileInfo);
    filestore->setDownloadInfo(downloadInfo);
    filestore->setSearchResults(searchResults);

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

