#ifndef PEERSTER_CHATDIALOG_HH
#define PEERSTER_CHATDIALOG_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>

#include "Peerster.hh"

class Peerster;
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
        QLineEdit* textline;
};

#endif // PEERSTER_CHATDIALOG_HH