#ifndef PEERSTER_CHATDIALOG_HH
#define PEERSTER_CHATDIALOG_HH

#include "Peerster.hh"

class Peerster;
class EntryQTextEdit;
class Message;
class Peer;

class ChatDialog : public QDialog
{
    Q_OBJECT

    public:
        ChatDialog(Peerster*);
        ~ChatDialog();
        void setTitle(QString);

    public slots:
        void gotReturnPressed();
        void gotDisplayMessage(Message);
        void gotNewPeerEntered();

    signals:
        void postToOutbox(Message);
        void potentialNewNeighbor(Peer);
        void sendStatusToPeer(Peer);

    private:
        Peerster* peerster;
        QTextEdit* textview;
        EntryQTextEdit* textentry;
        EntryQTextEdit* peerentry;
        QPushButton* addbtn;
        QString title;
};

class EntryQTextEdit : public QTextEdit 
{
    Q_OBJECT

    public:
        EntryQTextEdit();
        void keyPressEvent(QKeyEvent*);

    signals:
        void returnPressed();
};

#endif // PEERSTER_CHATDIALOG_HH