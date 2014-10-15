#include "GUI.hh"

GUI::GUI(Peerster* p)
    : peerster(p)
    , mainlayout(new QGridLayout(this))
    , chatlayout(new QVBoxLayout(this))
    , peerlayout(new QVBoxLayout(this))
    , dselectlayout(new QVBoxLayout(this))
    , dchatlayout(new QVBoxLayout(this))
    , chatview(new QTextEdit(this))
    , chatentry(new EntryQTextEdit())
    , peerview(new QTextEdit(this))
    , peerentry(new EntryQTextEdit())
    , dchatview(new QTextEdit(this))
    , dchatentry(new EntryQTextEdit())
    , addbtn(new QPushButton())
    , originslist(new QListWidget(this))
{
    connect(chatentry, SIGNAL(returnPressed()), 
        this, SLOT(gotReturnPressed()));
    connect(peerentry, SIGNAL(returnPressed()),
        this, SLOT(gotNewPeerEntered()));
    connect(addbtn, SIGNAL(clicked()),
        this, SLOT(gotNewPeerEntered()));
    connect(dchatentry, SIGNAL(returnPressed()),
        this, SLOT(gotNewDChatMsgEntered()));
    connect(originslist, SIGNAL(itemClicked(QListWidgetItem*)),
        this, SLOT(originSelected(QListWidgetItem*)));
    
    createPeerLayout();
    createChatLayout();
    createDirectLayout();

    mainlayout->addLayout(peerlayout, 0, 0);
    mainlayout->addLayout(chatlayout, 0, 1);
    mainlayout->addLayout(dselectlayout, 0, 2);
    mainlayout->addLayout(dchatlayout, 0, 3);

    setLayout(mainlayout);

    chatentry->setFocus();
}

GUI::~GUI()
{}

void GUI::setGroupConvo(QList<Message>* gc)
{
    groupConvo = gc;
}

void GUI::setDirectStore(QMap< QString,QList<Message> >* ds)
{
    directStore = ds;
}

void GUI::gotReturnPressed()
{
    // create Message
    Message msg;
    msg.setType(TYPE_RUMOR_CHAT);
    msg.setText(QString(chatentry->toPlainText()));

    // send to outbox
    Q_EMIT(postToOutbox(msg));

    // Clear the chatentry to get ready for the next input message.
    chatentry->clear();
}

void GUI::gotRefreshGroupConvo()
{
    foreach(Message msg, groupConvo)
    {
        chatview->append(msg.getOriginID() + 
            "<" + QString::number(msg.getSeqNo()) + ">: " + msg.getText());
    }
}

void GUI::gotNewPeerEntered()
{
    Peer peer = Peer(QString(peerentry->toPlainText()));

    Q_EMIT(potentialNewNeighbor(peer));
    Q_EMIT(sendStatusToPeer(peer));

    peerentry->clear();
}

void GUI::gotRefreshOrigins(QString newOrigin)
{   // change to full-list updates.
    qDebug() << "ADD NEW ORIGIN TO GUI LIST:" << newOrigin;
    new QListWidgetItem(newOrigin, originslist);
}

void GUI::gotDirectChatEntered()
{
    Message dmsg;

    dmsg.setType(TYPE_DIRECT_CHAT);
    dmsg.setDest(originslist->currentItem()->text());
    dmsg.setHopLimit(DCHAT_HOP_LIMIT);
    dmsg.setText(QString(dchatentry->toPlainText()));

    Q_EMIT(postToOutbox(dmsg));    

    dchatentry->clear();
}

void GUI::gotRefreshDirectConvo(QString origin)
{  
    dchatview->clear();

    QListWidgetItem* itm = originslist->currentItem();

    if(itm == NULL || originslist->currentItem()->text() != origin)
    // switch focus to updated convo
    {
        QList<QListWidgetItem*> originItems = 
                                originslist->findItems(origin, Qt::MatchExactly);
        if(originItems.size() != 1)
        {
            qDebug() << "Whoops! Multiple copies of orign: " << origin;
        }
        originslist->setCurrentItem(originItems.at(0));
    }
    
    QList<Message> convo = directStore->value(origin);

    QList<Message>::iterator i;
    for(i = history.begin(); i != history.end(); ++i)
    {
        dchatview->append(i->getOriginID() + ": " + i->getText());
    }

    dchatentry->setReadOnly(false);
    dchatentry->setFocus();
}

void GUI::gotRefreshNeighbors(QList<Peer> neighbors)
{
    peerview->clear();

    QList<Peer>::iterator i;
    for(i = neighbors.begin(); i != neighbors.end(); ++i)
    {
        peerview->append(i->toString());
    }
}

void GUI::originSelected(QListWidgetItem* item)
{
    Q_EMIT(getDChatHistoryFromOrigin(item->text()));

    dchatentry->setReadOnly(false);
    dchatentry->setFocus();
}

void GUI::createChatLayout()
{
    QLabel* chatlabel = new QLabel(TITLE_CHAT, this, 0);

    chatview->setReadOnly(true);

    chatentry->setReadOnly(false);
    chatentry->setLineWrapMode(QTextEdit::WidgetWidth);

    chatlayout->addWidget(chatlabel);
    chatlayout->addWidget(chatview);
    chatlayout->addWidget(chatentry);
}

void GUI::createPeerLayout()
{
    QLabel* peerlabel = new QLabel(TITLE_PEER, this, 0);

    peerview->setReadOnly(true);

    peerentry->setReadOnly(false);
    peerentry->setLineWrapMode(QTextEdit::WidgetWidth);

    addbtn->setText(TITLE_ADDPEER);

    peerlayout->addWidget(peerlabel);
    peerlayout->addWidget(peerview);
    peerlayout->addWidget(peerentry);
    peerlayout->addWidget(addbtn);
}

void GUI::createDirectLayout()
{
    QLabel* dselectlabel = new QLabel(TITLE_DSELECT, this, 0);

    dselectlayout->addWidget(dselectlabel);
    dselectlayout->addWidget(originslist);


    QLabel* dchatlabel = new QLabel(TITLE_DCHAT, this, 0);
    dchatview->setReadOnly(true);
    dchatentry->setReadOnly(true);
    dchatentry->setLineWrapMode(QTextEdit::WidgetWidth);

    dchatlayout->addWidget(dchatlabel);
    dchatlayout->addWidget(dchatview);
    dchatlayout->addWidget(dchatentry);
}

EntryQTextEdit::EntryQTextEdit()
{}

void EntryQTextEdit::keyPressEvent(QKeyEvent* event) 
{
    if(event->key()==Qt::Key_Return)
    {
        Q_EMIT(returnPressed());
    }
    else
    {
        QTextEdit::keyPressEvent(event);
    }
}

