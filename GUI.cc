#include "GUI.hh"

GUI::GUI(Peerster* p)
    : peerster(p)
    , mainlayout(new QGridLayout(this))
    , filelayout(new QVBoxLayout(this))
    , groupchatlayout(new QVBoxLayout(this))
    , neighborlayout(new QVBoxLayout(this))
    , originslayout(new QVBoxLayout(this))
    , directchatlayout(new QVBoxLayout(this))
    , fileview(new QTextEdit(this))
    , groupchatview(new QTextEdit(this))
    , neighborview(new QTextEdit(this))
    , directchatview(new QTextEdit(this))
    , groupchatentry(new EntryQTextEdit())
    , directchatentry(new EntryQTextEdit())
    , neighborentry(new EntryQTextEdit())
    , addneighborbutton(new QPushButton())
    , addfilebutton(new QPushButton())
    , originslist(new QListWidget(this))
{
    connect(groupchatentry, SIGNAL(returnPressed()), 
        this, SLOT(gotGroupChatEntered()));
    connect(neighborentry, SIGNAL(returnPressed()),
        this, SLOT(gotNeighborEntered()));
    connect(addneighborbutton, SIGNAL(clicked()),
        this, SLOT(gotNeighborEntered()));
    connect(addfilebutton, SIGNAL(clicked()),
        this, SLOT(gotOpenFileDialog()));
    connect(directchatentry, SIGNAL(returnPressed()),
        this, SLOT(gotDirectChatEntered()));
    connect(this, SIGNAL(refreshDirectConvo(QString)),
        this, SLOT(gotRefreshDirectConvo(QString)));
    connect(originslist, SIGNAL(itemClicked(QListWidgetItem*)),
        this, SLOT(originSelected(QListWidgetItem*)));
    
    createFileLayout();
    createNeighborLayout();
    createGroupChatLayout();
    createOriginsLayout();
    createDirectChatLayout();

    mainlayout->addLayout(filelayout,       0, 0);
    mainlayout->addLayout(neighborlayout,   1, 0);
    mainlayout->addLayout(groupchatlayout,  0, 1, 2, 1);
    mainlayout->addLayout(originslayout,    0, 2);
    mainlayout->addLayout(directchatlayout, 1, 2);

    setLayout(mainlayout);

    groupchatentry->setFocus();
}

GUI::~GUI()
{}

void GUI::setID(QString id)
{
    ID = id;
}

void GUI::setSharedFileInfo(QMap<QString,quint32>* sfi)
{
    sharedFileInfo = sfi;
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
    groupchatview->clear();
    foreach(Message msg, *groupConvo)
    {
        groupchatview->append(msg.getOriginID() + 
            "<" + QString::number(msg.getSeqNo()) + ">: " + msg.getText());
    }
}

void GUI::gotRefreshDirectConvo(QString origin)
{  
    directchatview->clear();

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
        directchatview->append(msg.getOriginID() + ": " + msg.getText());
    }

    directchatentry->setReadOnly(false);
    directchatentry->setFocus();
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
        }
    }
}

void GUI::gotRefreshNeighbors(QList<Peer> neighbors)
{ // TODO: change input to QStringList
    neighborview->clear();

    foreach(Peer neighbor, neighbors)
    {
        neighborview->append(neighbor.toString());
    }
}

void GUI::originSelected(QListWidgetItem* item)
{
    Q_EMIT(refreshDirectConvo(item->text()));
}

void GUI::gotGroupChatEntered()
{
    Q_EMIT(createChatRumor(groupchatentry->toPlainText()));
    groupchatentry->clear();
}

void GUI::gotDirectChatEntered()
{
    Q_EMIT(createDirectChat(originslist->currentItem()->text(),
                            directchatentry->toPlainText()));    
    directchatentry->clear();
}

void GUI::gotNeighborEntered()
{
    Peer peer = Peer(QString(neighborentry->toPlainText()));

    Q_EMIT(processNeighbor(peer));

    neighborentry->clear();
}

void GUI::gotOpenFileDialog()
{
    QStringList fileNames = 
        QFileDialog::getOpenFileNames(this, tr("Share File"), "~/");

    qDebug() << "FILES SELECTED:";
    foreach(QString str, fileNames)
    {
        qDebug() << str;
    }
}

void GUI::createFileLayout()
{
    QLabel* filelabel = new QLabel(TITLE_FILE, this, 0);

    fileview->setReadOnly(true);

    addfilebutton->setText(TITLE_ADDFILE);

    filelayout->addWidget(filelabel);
    filelayout->addWidget(fileview);
    filelayout->addWidget(addfilebutton);
}

void GUI::createNeighborLayout()
{
    QLabel* neighborlabel = new QLabel(TITLE_NEIGHBOR, this, 0);

    neighborview->setReadOnly(true);

    neighborentry->setReadOnly(false);
    neighborentry->setLineWrapMode(QTextEdit::WidgetWidth);

    addneighborbutton->setText(TITLE_ADDNEIGHBOR);

    neighborlayout->addWidget(neighborlabel);
    neighborlayout->addWidget(neighborview);
    neighborlayout->addWidget(neighborentry);
    neighborlayout->addWidget(addneighborbutton);
}

void GUI::createGroupChatLayout()
{
    QLabel* groupchatlabel = new QLabel(TITLE_GROUPCHAT, this, 0);

    groupchatview->setReadOnly(true);

    groupchatentry->setReadOnly(false);
    groupchatentry->setLineWrapMode(QTextEdit::WidgetWidth);

    groupchatlayout->addWidget(groupchatlabel);
    groupchatlayout->addWidget(groupchatview);
    groupchatlayout->addWidget(groupchatentry);
}

void GUI::createOriginsLayout()
{
    QLabel* originslabel = new QLabel(TITLE_ORIGINS, this, 0);

    originslayout->addWidget(originslabel);
    originslayout->addWidget(originslist);
}

void GUI::createDirectChatLayout()
{
    QLabel* directchatlabel = new QLabel(TITLE_DIRECTCHAT, this, 0);

    directchatview->setReadOnly(true);

    directchatentry->setReadOnly(true);
    directchatentry->setLineWrapMode(QTextEdit::WidgetWidth);

    directchatlayout->addWidget(directchatlabel);
    directchatlayout->addWidget(directchatview);
    directchatlayout->addWidget(directchatentry);
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

