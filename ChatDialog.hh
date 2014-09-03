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

    public slots:
        void gotReturnPressed();

    private:
        Peerster* peerster;
        QTextEdit* textview;
        EntryQTextEdit* textline;
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