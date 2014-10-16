#include "GUI.hh"

GUI::GUI(Peerster* p)
    : peerster(p)
    , mainlayout(new QGridLayout(this))
    , chatlayout(new QVBoxLayout(this))
    , peerlayout(new QVBoxLayout(this))
    , dselectlayout(new QVBoxLayout(this))
    , dchatlayout(new QVBoxLayout(this))
    , chatview(new QTextEdit(this))
    , peerview(new QTextEdit(this))
    , dchatview(new QTextEdit(this))
    , addbtn(new QPushButton())
    , originslist(new QListWidget(this))
    , chatentry(new EntryQTextEdit())
    , dchatentry(new EntryQTextEdit())
    , peerentry(new EntryQTextEdit())
{
    connect(chatentry, SIGNAL(returnPressed()), 
        this, SLOT(gotGroupChatEntered()));
    connect(peerentry, SIGNAL(returnPressed()),
        this, SLOT(gotNeighborEntered()));
    connect(addbtn, SIGNAL(clicked()),
        this, SLOT(gotNeighborEntered()));
    connect(dchatentry, SIGNAL(returnPressed()),
        this, SLOT(gotDirectChatEntered()));
    connect(this, SIGNAL(refreshDirectConvo(QString)),
        this, SLOT(gotRefreshDirectConvo(QString)));

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

void GUI::setID(QString id)
{
    ID = id;
}

void GUI::setGroupConvo(QList<Message>* gc)
{
    groupConvo = gc;
}

void GUI::setDirectStore(QMap< QString,QList<Message> >* ds)
{
    directStore = ds;
}

void GUI::gotRefreshGroupConvo()
{
    qDebug() << "groupConvo SIZE: " << groupConvo->size();
    chatview->clear();
    foreach(Message msg, *groupConvo)
    {
        chatview->append(msg.getOriginID() + 
            "<" + QString::number(msg.getSeqNo()) + ">: " + msg.getText());
    }
}

void GUI::gotRefreshDirectConvo(QString origin)
{  
    dchatview->clear();

    QListWidgetItem* itm = originslist->currentItem();

    if(itm == NULL || originslist->currentItem()->text() != origin)
    {   // switch focus to updated convo
        QList<QListWidgetItem*> originItems = 
                                originslist->findItems(origin, Qt::MatchExactly);
        if(originItems.size() != 1)
        {
            qDebug() << "Whoops! Multiple copies of orign: " << origin;
        }
        originslist->setCurrentItem(originItems.at(0));
    }
    
    QList<Message> convo = directStore->value(origin);

    foreach(Message msg, convo)
    {
        dchatview->append(msg.getOriginID() + ": " + msg.getText());
    }

    dchatentry->setReadOnly(false);
    dchatentry->setFocus();
}

void GUI::gotRefreshOrigins(QStringList origins)
{
    QStringList originsListContents = QStringList();

    for(int i = 0; i < originslist->count(); i++)
    {
        if(!origins.contains(originslist->item(i)->text()))
        {
            delete(originslist->takeItem(i));
        }
        else
        {
            originsListContents.append(originslist->item(i)->text());
        }
    }
    foreach(QString origin, origins)
    {
        if(!originsListContents.contains(origin) && origin != ID)
        {
            new QListWidgetItem(origin, originslist);
            qDebug() << "ADD NEW ORIGIN TO GUI LIST:" << origins.last();
        }
    }
}

void GUI::gotRefreshNeighbors(QList<Peer> neighbors)
{ // TODO: change input to QStringList
    peerview->clear();

    foreach(Peer neighbor, neighbors)
    {
        peerview->append(neighbor.toString());
    }
}

void GUI::originSelected(QListWidgetItem* item)
{
    Q_EMIT(refreshDirectConvo(item->text()));
}

void GUI::gotGroupChatEntered()
{
    Q_EMIT(createChatRumor(chatentry->toPlainText()));
    chatentry->clear();
}

void GUI::gotDirectChatEntered()
{
    Q_EMIT(createDirectChat(originslist->currentItem()->text(),
                            dchatentry->toPlainText()));    
    dchatentry->clear();
}

void GUI::gotNeighborEntered()
{
    Peer peer = Peer(QString(peerentry->toPlainText()));

    Q_EMIT(processNeighbor(peer));

    peerentry->clear();
}

void GUI::clearOriginsList()
{
    while(originslist->count() > 0)
    {
      delete(originslist->takeItem(0));
    }
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

