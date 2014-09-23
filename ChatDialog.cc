#include "ChatDialog.hh"

ChatDialog::ChatDialog(Peerster* p)
    : peerster(p)
    , textview(new QTextEdit(this))
    , textentry(new EntryQTextEdit())
    , peerentry(new EntryQTextEdit())
    , addbtn(new QPushButton())
{
    connect(textentry, SIGNAL(returnPressed()), 
        this, SLOT(gotReturnPressed()));
    connect(peerentry, SIGNAL(returnPressed()),
        this, SLOT(gotNewPeerEntered()));
    connect(addbtn, SIGNAL(clicked()),
        this, SLOT(gotNewPeerEntered()));

    // Read-only text box where we display messages from everyone.
    // This widget expands both horizontally and vertically.
    textview->setReadOnly(true);

    // Small text-entry box the user can enter messages.
    // L1E2: multi-line word-wrapped text entry box
    textentry->setReadOnly(false);
    textentry->setLineWrapMode(QTextEdit::WidgetWidth);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textentry);

    peerentry->setReadOnly(false);
    peerentry->setLineWrapMode(QTextEdit::WidgetWidth);
    layout->addWidget(peerentry);
    addbtn->setText("Add Peer");
    layout->addWidget(addbtn);
    
    setLayout(layout);

    // L1E1: set line focus to textentry on startup
    textentry->setFocus();
}

ChatDialog::~ChatDialog()
{}

void ChatDialog::setTitle(QString str)
{
    title = str;
    setWindowTitle(title);
}

void ChatDialog::gotReturnPressed()
{
    // create Message
    Message msg;
    msg.setType(TYPE_RUMOR_CHAT);
    msg.setText(QString(textentry->toPlainText()));

    // send to outbox
    Q_EMIT(postToOutbox(msg));

    // Clear the textentry to get ready for the next input message.
    textentry->clear();
}

void ChatDialog::gotDisplayMessage(Message msg)
{
    textview->append(msg.getOriginID() + 
        "<" + QString::number(msg.getSeqNo()) + ">: " + msg.getText());
}

void ChatDialog::gotNewPeerEntered()
{
    Peer peer = Peer(QString(peerentry->toPlainText()));

    Q_EMIT(potentialNewNeighbor(peer));
    Q_EMIT(sendStatusToPeer(peer));

    peerentry->clear();
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

