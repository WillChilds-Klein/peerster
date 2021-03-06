#include "GUI.hh"

GUI::GUI(Peerster* p)
    : peerster(p)
    , mainlayout(new QGridLayout(this))
    , filesharelayout(new QVBoxLayout(this))
    , filesearchlayout(new QVBoxLayout(this))
    , groupchatlayout(new QVBoxLayout(this))
    , neighborlayout(new QVBoxLayout(this))
    , originslayout(new QVBoxLayout(this))
    , directchatlayout(new QVBoxLayout(this))
    , fileshareview(new QTextEdit(this))
    , downloadsview(new QTextEdit(this))
    , groupchatview(new QTextEdit(this))
    , neighborview(new QTextEdit(this))
    , directchatview(new QTextEdit(this))
    , filesearchentry(new EntryQTextEdit())
    , groupchatentry(new EntryQTextEdit())
    , directchatentry(new EntryQTextEdit())
    , neighborentry(new EntryQTextEdit())
    , addneighborbutton(new QPushButton())
    , sharefilebutton(new QPushButton())
    , originslist(new QListWidget(this))
    , searchresultlist(new QListWidget(this))
{
    connect(groupchatentry, SIGNAL(returnPressed()), 
        this, SLOT(gotGroupChatEntered()));
    connect(neighborentry, SIGNAL(returnPressed()),
        this, SLOT(gotNeighborEntered()));
    connect(addneighborbutton, SIGNAL(clicked()),
        this, SLOT(gotNeighborEntered()));
    connect(sharefilebutton, SIGNAL(clicked()),
        this, SLOT(gotOpenFileDialog()));
    connect(directchatentry, SIGNAL(returnPressed()),
        this, SLOT(gotDirectChatEntered()));
    connect(this, SIGNAL(refreshDirectConvo(QString)),
        this, SLOT(gotRefreshDirectConvo(QString)));
    connect(originslist, SIGNAL(itemClicked(QListWidgetItem*)),
        this, SLOT(originSelected(QListWidgetItem*)));
    connect(filesearchentry, SIGNAL(returnPressed()),
        this, SLOT(gotFileSearchEntered()));
    connect(searchresultlist, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
        this, SLOT(searchresultSelected(QListWidgetItem*)));
    
    createFileShareLayout();
    createFileSearchLayout();
    createGroupChatLayout();
    createOriginsLayout();
    createNeighborLayout();
    createDirectChatLayout();

    mainlayout->addLayout(filesharelayout,  0, 0);
    mainlayout->addLayout(filesearchlayout, 1, 0);
    mainlayout->addLayout(groupchatlayout,  0, 1);
    mainlayout->addLayout(originslayout,    1, 1);
    mainlayout->addLayout(neighborlayout,   0, 2);
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

void GUI::setDownloadInfo(QMap<QString,DownloadStatus::Status>* di)
{
    downloadInfo = di;
}

void GUI::setSearchResults(QMultiHash< QString,QPair<QString,QByteArray> >* sr)
{
    searchResults = sr;
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
{ 
    neighborview->clear();

    foreach(Peer neighbor, neighbors)
    {
        neighborview->append(neighbor.toString());
    }
}

void GUI::gotRefreshSharedFiles()
{
    fileshareview->clear();

    QString entry;
    foreach(QString filename, sharedFileInfo->keys())
    {
        entry = filename + "(" + sharedFileInfo->value(filename) + "B)";
        entry += "\n------------------------------";
        fileshareview->append(filename);
    }
}

void GUI::gotRefreshDownloadInfo()
{
    downloadsview->clear();

    QString download;
    foreach(QString filename, downloadInfo->keys())
    {
        download = filename + "\t(" +  
            DownloadStatus::statusString[downloadInfo->value(filename)] + ")";
        downloadsview->append(download);
    }   
}

void GUI::gotRefreshSearchResults()
{
    QString result;
    searchresultlist->clear();
    QMultiHash< QString,QPair<QString,QByteArray> >::iterator i;
    for(i = searchResults->begin(); i != searchResults->end(); ++i)
    {
        result = i.value().first + "\t(peer: " + i.key() + ")";
        new QListWidgetItem(result, searchresultlist);
    }
}

void GUI::gotStartSearch()
{
    filesearchentry->setReadOnly(true);
}

void GUI::gotEndSearch()
{
    filesearchentry->setReadOnly(false);
    QListWidgetItem* item = new QListWidgetItem("SEARCH COMPLETED", searchresultlist);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
}

void GUI::originSelected(QListWidgetItem* item)
{
    Q_EMIT(refreshDirectConvo(item->text()));
}

void GUI::searchresultSelected(QListWidgetItem* item)
{
    QPair<QString,QByteArray> match;
    QString formatted;

    QMultiHash< QString,QPair<QString,QByteArray> >::iterator i;
    for(i = searchResults->begin(); i != searchResults->end(); ++i)
    {
        formatted = i.value().first + "\t(peer: " + i.key() + ")";
        if(formatted == item->text())
        {
            Q_EMIT(requestFileFromPeer(i.key(), i.value()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            return;
        }
    }

    QString errormsg = "WHOOPS! SELECTED SEARCH RESULT NOT IN TABLE!";
    new QListWidgetItem(errormsg, searchresultlist);
    qDebug() << errormsg;
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

    Q_EMIT(processFilesToShare(fileNames));
}

void GUI::gotFileSearchEntered()
{
    QStringList keys = searchResults->keys();
    foreach(QString key, keys)
    {
        searchResults->remove(key);
    }
    searchresultlist->clear();

    QString keywords = QString(filesearchentry->toPlainText());
    Q_EMIT(searchForKeywords(keywords));

    filesearchentry->clear();
}

void GUI::createFileShareLayout()
{
    QLabel* filesharelabel = new QLabel(TITLE_FILESHARE, this, 0);
    QLabel* downloadslabel = new QLabel(TITLE_DOWNLOADS, this, 0);

    fileshareview->setReadOnly(true);

    sharefilebutton->setText(TITLE_SHAREFILE);

    downloadsview->setReadOnly(true);

    filesharelayout->addWidget(filesharelabel);
    filesharelayout->addWidget(fileshareview);
    filesharelayout->addWidget(sharefilebutton);
    filesharelayout->addWidget(downloadslabel);
    filesharelayout->addWidget(downloadsview);
}

void GUI::createFileSearchLayout()
{
    QLabel* filesearchlabel = new QLabel(TITLE_FILESEARCH, this, 0);

    filesearchentry->setReadOnly(false);
    filesearchentry->setLineWrapMode(QTextEdit::WidgetWidth);

    filesearchlayout->addWidget(filesearchlabel);
    filesearchlayout->addWidget(searchresultlist);
    filesearchlayout->addWidget(filesearchentry);
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

