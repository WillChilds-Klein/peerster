#ifndef PEERSTER_GUI_HH
#define PEERSTER_GUI_HH

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

class GUI : public QDialog
{
    Q_OBJECT

    public:
        GUI(Peerster*);
        ~GUI();
        void setGroupConvo(QList<Message>*);
        void setDirectStore(QMap<QString,QList<Message> >*);

    public slots:
        void gotGroupChatEntered();
        void gotDirectChatEntered();
        void gotNeighborEntered();

        void gotRefreshGroupConvo();
        void gotRefreshDirectConvo(QString);
        void gotRefreshNeighbors(QList<Peer>);
        void gotRefreshOrigins(QString);

    private slots:
        void originSelected(QListWidgetItem*);

    signals:
        void postToOutbox(Message);
        void processNeighbor(Peer);

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* directStore;
        QList<Message>* groupConvo;
        QGridLayout* mainlayout;
        QVBoxLayout *chatlayout, *peerlayout, *dselectlayout, *dchatlayout;
        QHBoxLayout *directlayout;
        QTextEdit *peerview, *chatview, *dchatview;
        EntryQTextEdit *chatentry, *peerentry, *dchatentry;
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

#endif // PEERSTER_GUI_HH