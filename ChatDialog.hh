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
        ChatDialog(Peerster* p);
        ~ChatDialog();
        QTextEdit* getTextview();
        EntryQTextEdit* getTextentry();
        void displayMessage(Message msg, bool fromMe);


    public slots:
        void gotReturnPressed();

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
        void keyPressEvent(QKeyEvent* event);

    signals:
        void returnPressed();
};

#endif // PEERSTER_CHATDIALOG_HH