#ifndef PEERSTER_CHATDIALOG_HH
#define PEERSTER_CHATDIALOG_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>

class ChatDialog : public QDialog
{
    Q_OBJECT

    public:
        ChatDialog();
        ~ChatDialog();

    public slots:
        void gotReturnPressed();

    private:
        QTextEdit* textview;
        QLineEdit* textline;
};

#endif // PEERSTER_CHATDIALOG_HH