#include "ChatDialog.hh"

ChatDialog::ChatDialog(Peerster* p)
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

ChatDialog::~ChatDialog()
{}

void ChatDialog::gotReturnPressed()
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

void ChatDialog::gotDisplayMessage(Message msg)
{
    chatview->append(msg.getOriginID() + 
        "<" + QString::number(msg.getSeqNo()) + ">: " + msg.getText());
}

void ChatDialog::gotNewPeerEntered()
{
    Peer peer = Peer(QString(peerentry->toPlainText()));

    Q_EMIT(potentialNewNeighbor(peer));
    Q_EMIT(sendStatusToPeer(peer));

    peerentry->clear();
}

void ChatDialog::gotUpdateGUIOriginsList(QString newOrigin)
{
    qDebug() << "ADD NEW ORIGIN TO GUI LIST:" << newOrigin;
    new QListWidgetItem(newOrigin, originslist);
}

void ChatDialog::gotNewDChatMsgEntered()
{
    Message dmsg;

    dmsg.setType(TYPE_DIRECT_CHAT);
    dmsg.setDest(originslist->currentItem()->text());
    dmsg.setHopLimit(DCHAT_HOP_LIMIT);
    dmsg.setText(QString(dchatentry->toPlainText()));

    Q_EMIT(postToOutbox(dmsg));    

    dchatentry->clear();
}

void ChatDialog::gotUpdateGUIDChatHistory(QString origin, QList<Message> history)
{  
    QListWidgetItem* itm = originslist->currentItem();

    if(itm == NULL || originslist->currentItem()->text() != origin)
    {
        QList<QListWidgetItem*> originItems = 
                                originslist->findItems(origin, Qt::MatchExactly);
        if(originItems.size() != 1)
        {
            qDebug() << "Whoops! Multiple copies of orign: " << origin;
        }
        originslist->setCurrentItem(originItems.at(0));
    }
        
    updateDChatView(history);

    dchatentry->setReadOnly(false);
    dchatentry->setFocus();
}

void ChatDialog::gotUpdateGUINeighbors(QList<Peer> neighbors)
{
    peerview->clear();

    QList<Peer>::iterator i;
    for(i = neighbors.begin(); i != neighbors.end(); ++i)
    {
        peerview->append(i->toString());
    }
}

void ChatDialog::originSelected(QListWidgetItem* item)
{
    Q_EMIT(getDChatHistoryFromOrigin(item->text()));

    dchatentry->setReadOnly(false);
    dchatentry->setFocus();
}

void ChatDialog::createChatLayout()
{
    QLabel* chatlabel = new QLabel(TITLE_CHAT, this, 0);

    chatview->setReadOnly(true);

    chatentry->setReadOnly(false);
    chatentry->setLineWrapMode(QTextEdit::WidgetWidth);

    chatlayout->addWidget(chatlabel);
    chatlayout->addWidget(chatview);
    chatlayout->addWidget(chatentry);
}

void ChatDialog::createPeerLayout()
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

void ChatDialog::createDirectLayout()
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

void ChatDialog::updateDChatView(QList<Message> history)
{
    dchatview->clear();

    QList<Message>::iterator i;
    for(i = history.begin(); i != history.end(); ++i)
    {
        dchatview->append(i->getOriginID() + ": " + i->getText());
    }
}

// L1E2: subclass QTextEdit to get desired UI behavior.
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

