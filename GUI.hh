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
        void gotReturnPressed();
        void gotNewPeerEntered();
        void gotUpdateGUIOriginsList(QString);

        void gotDirectChatEntered();
        void gotRefresheighbors(QList<Peer>);
        void gotRefreshGroupConvo();
        void gotRefrshOrigins(QString);

        // to implement.
        void gotRefreshDirectConvo(QString);
        void gotRefreshNeighbors(QStringList);

    private slots:
        void originSelected(QListWidgetItem*);

    signals:
        void postToOutbox(Message);
        void potentialNewNeighbor(Peer);
        void sendStatusToPeer(Peer);
        void getDChatHistoryFromOrigin(QString qstr);

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