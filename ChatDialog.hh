#ifndef PEERSTER_CHATDIALOG_HH
#define PEERSTER_CHATDIALOG_HH

#include "Peerster.hh"

class Peerster;
class EntryQTextEdit;
class Message;

class ChatDialog : public QDialog
{
    Q_OBJECT

    public:
        ChatDialog(Peerster*);
        ~ChatDialog();
        QTextEdit* getTextview();
        EntryQTextEdit* getTextentry();
        void displayMessage(Message, bool);


    public slots:
        void gotReturnPressed();
        void gotDisplayMessage(Message);

    signals:
        void postToOutbox(Message);
        void displayMessage(Message);

    private:
        Peerster* peerster;
        QTextEdit* textview;
        EntryQTextEdit* textentry;
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