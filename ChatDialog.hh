#ifndef PEERSTER_CHATDIALOG_HH
#define PEERSTER_CHATDIALOG_HH

#define TITLE_PEER ("Neighbor Peers")
#define TITLE_CHAT ("Chat Messages")
#define TITLE_DSELECT ("Origin ID's Available for DM")
#define TITLE_DCHAT ("Direct Messages")

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
        void gotNewDChatMsgEntered();
        void gotUpdateGUIDChatHistory(QString,QList<Message>);

    private slots:
        void originSelected(QListWidgetItem*);

    signals:
        void postToOutbox(Message);
        void potentialNewNeighbor(Peer);
        void sendStatusToPeer(Peer);
        void getDChatHistoryFromOrigin(QString qstr);

    private:
        Peerster* peerster;
        QGridLayout* mainlayout;
        QVBoxLayout *chatlayout, *peerlayout, *dselectlayout, *dchatlayout;
        QHBoxLayout *directlayout;
        QTextEdit *chatview, *dchatview;
        EntryQTextEdit *chatentry, *peerentry, *dchatentry;
        QPushButton* addbtn;
        QListWidget* originslist;
        void createChatLayout();
        void createPeerLayout();
        void createDirectLayout();
        void updateDChatView(QList<Message>);
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