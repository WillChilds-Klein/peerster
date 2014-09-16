#include "ChatDialog.hh"

ChatDialog::ChatDialog(Peerster* p)
    : peerster(p)
    , textview(new QTextEdit(this))
    , textentry(new EntryQTextEdit())
{
    // Register a callback on the textentry's returnPressed signal
    // so that we can send the message entered by the user.
    connect(textentry, SIGNAL(returnPressed()), 
        this, SLOT(gotReturnPressed()));

    // Read-only text box where we display messages from everyone.
    // This widget expands both horizontally and vertically.
    textview->setReadOnly(true);

    // Small text-entry box the user can enter messages.
    // L1E2: multi-line word-wrapped text entry box
    textentry->setReadOnly(false);
    textentry->setLineWrapMode(QTextEdit::WidgetWidth);

    // Lay out the widgets to appear in the main window.
    // For Qt widget and layout concepts see:
    // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textentry);
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

// L1E2: subclass QTextEdit to get desired UI behavior.
EntryQTextEdit::EntryQTextEdit() : QTextEdit() {}

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

