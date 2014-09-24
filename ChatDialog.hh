#ifndef PEERSTER_CHATDIALOG_HH
#define PEERSTER_CHATDIALOG_HH

#define TITLE_PEER ("Neighbor Peers")
#define TITLE_CHAT ("Chat Messages")
#define TITLE_DIRECT ("Origin ID's Available for DM")

#define TITLE_ADDPEER ("Add Neighbor")

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

    public slots:
        void gotReturnPressed();
        void gotDisplayMessage(Message);
        void gotNewPeerEntered();
        void gotUpdateGUIOriginsList(QString);

    signals:
        void postToOutbox(Message);
        void potentialNewNeighbor(Peer);
        void sendStatusToPeer(Peer);

    private:
        Peerster* peerster;
        QGridLayout* mainlayout;
        QVBoxLayout *chatlayout, *peerlayout, *directlayout;
        QTextEdit* chatview;
        EntryQTextEdit *chatentry, *peerentry;
        QPushButton* addbtn;
        QListWidget* originslist;
        void createChatLayout();
        void createPeerLayout();
        void createDirectLayout();
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