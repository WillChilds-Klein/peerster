#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "ChatDialog.hh"

ChatDialog::ChatDialog(Peerster* p)
    : peerster(p)
{
    setWindowTitle("Peerster");

    // Read-only text box where we display messages from everyone.
    // This widget expands both horizontally and vertically.
    textview = new QTextEdit(this);
    textview->setReadOnly(true);

    // Small text-entry box the user can enter messages.
    // L1E2: multi-line word-wrapped text entry box
    textline = new EntryQTextEdit();
    textline->setReadOnly(false);
    textline->setLineWrapMode(QTextEdit::WidgetWidth);

    // Lay out the widgets to appear in the main window.
    // For Qt widget and layout concepts see:
    // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textline);
    setLayout(layout);

    // Register a callback on the textline's returnPressed signal
    // so that we can send the message entered by the user.
    connect(textline, SIGNAL(returnPressed()),
        this, SLOT(gotReturnPressed()));

    // L1E1: set line focus to textline on startup
    textline->setFocus();
}

ChatDialog::~ChatDialog() {}

void ChatDialog::gotReturnPressed()
{
    // Initially, just echo the string locally.
    // Insert some networking code here...
    qDebug() << "FIX: send message to other peers: " << textline->toPlainText();
    textview->append(textline->toPlainText());

    // Clear the textline to get ready for the next input message.
    textline->clear();
}

// L1E2: subclass QTextEdit to get desired UI behavior.
EntryQTextEdit::EntryQTextEdit() : QTextEdit() {}

void EntryQTextEdit::keyPressEvent(QKeyEvent* event) 
{
    if(event->key()==Qt::Key_Return){
        Q_EMIT(returnPressed());
    }
    else {
        QTextEdit::keyPressEvent(event);
    }
}