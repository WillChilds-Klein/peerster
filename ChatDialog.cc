#include "ChatDialog.hh"

ChatDialog::ChatDialog(Peerster* p)
    : peerster(p)
    , mainlayout(new QGridLayout(this))
    , chatlayout(new QVBoxLayout(this))
    , peerlayout(new QVBoxLayout(this))
    , directlayout(new QVBoxLayout(this))
    , chatview(new QTextEdit(this))
    , chatentry(new EntryQTextEdit())
    , peerentry(new EntryQTextEdit())
    , addbtn(new QPushButton())
    , originslist(new QListWidget(this))
{
    connect(chatentry, SIGNAL(returnPressed()), 
        this, SLOT(gotReturnPressed()));
    connect(peerentry, SIGNAL(returnPressed()),
        this, SLOT(gotNewPeerEntered()));
    connect(addbtn, SIGNAL(clicked()),
        this, SLOT(gotNewPeerEntered()));
    
    createPeerLayout();
    createChatLayout();
    createDirectLayout();

    mainlayout->addLayout(peerlayout, 0, 0);
    mainlayout->addLayout(chatlayout, 0, 1);
    mainlayout->addLayout(directlayout, 0, 2);

    setLayout(mainlayout);
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
    // newItem->setText(newOrigin);
    // originslist->addItem(newItem);
}

void ChatDialog::createChatLayout()
{
    QLabel* chatlabel = new QLabel(TITLE_CHAT, this, 0);

    chatview->setReadOnly(true);

    chatentry->setReadOnly(false);
    chatentry->setLineWrapMode(QTextEdit::WidgetWidth);
    chatentry->setFocus();

    chatlayout->addWidget(chatlabel);
    chatlayout->addWidget(chatview);
    chatlayout->addWidget(chatentry);
}

void ChatDialog::createPeerLayout()
{
    QLabel* peerlabel = new QLabel(TITLE_PEER, this, 0);

    peerentry->setReadOnly(false);
    peerentry->setLineWrapMode(QTextEdit::WidgetWidth);

    addbtn->setText(TITLE_ADDPEER);

    peerlayout->addWidget(peerlabel);
    peerlayout->addWidget(peerentry);
    peerlayout->addWidget(addbtn);
}

void ChatDialog::createDirectLayout()
{
    QLabel* directlabel = new QLabel(TITLE_DIRECT, this, 0);

    directlayout->addWidget(directlabel);
    directlayout->addWidget(originslist);
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

