#ifndef PEERSTER_GUI_HH
#define PEERSTER_GUI_HH

#define TITLE_FILE ("Shared Files")
#define TITLE_NEIGHBOR ("Neighbor Peer Locations")
#define TITLE_GROUPCHAT ("Group Chat")
#define TITLE_ORIGINS ("Known Peer ID's")
#define TITLE_DIRECTCHAT ("Direct Messages")

#define TITLE_ADDFILE ("Add File")
#define TITLE_ADDNEIGHBOR ("Add Neighbor")

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
        void setID(QString);
        void setSharedFileInfo(QMap<QString,quint32>*);
        void setGroupConvo(QList<Message>*);
        void setDirectStore(QMap<QString,QList<Message> >*);

    signals:
        void processNeighbor(Peer);
        void processShareFiles(QStringList);
        void createChatRumor(QString);
        void createDirectChat(QString,QString);
        void refreshDirectConvo(QString);

    public slots:
        void gotRefreshGroupConvo();
        void gotRefreshDirectConvo(QString);
        void gotRefreshOrigins(QStringList);
        void gotRefreshNeighbors(QList<Peer>);

    private slots:
        void originSelected(QListWidgetItem*);
        void gotGroupChatEntered();
        void gotDirectChatEntered();
        void gotNeighborEntered();
        void gotOpenFileDialog();

    private:
        Peerster* peerster;
        QMap<QString, quint32>* sharedFileInfo;
        QMap< QString, QList<Message> >* directStore;
        QList<Message>* groupConvo;
        QString ID;
        QGridLayout* mainlayout;
        QVBoxLayout *groupchatlayout, *neighborlayout, 
                    *directchatlayout, *originslayout, *filelayout;
        QTextEdit *neighborview, *groupchatview, 
                  *directchatview, *fileview;
        EntryQTextEdit *groupchatentry, *neighborentry, 
                       *directchatentry;
        QPushButton *addneighborbutton, *addfilebutton;
        QListWidget* originslist;
        void createFileLayout();
        void createNeighborLayout();
        void createGroupChatLayout();
        void createOriginsLayout();
        void createDirectChatLayout();
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