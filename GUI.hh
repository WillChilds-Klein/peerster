#ifndef PEERSTER_GUI_HH
#define PEERSTER_GUI_HH

#define TITLE_FILESHARE ("Shared Files")
#define TITLE_DOWNLOADS ("Downloads")
#define TITLE_FILESEARCH ("Search Files")
#define TITLE_NEIGHBOR ("Neighbor Peer Locations")
#define TITLE_GROUPCHAT ("Group Chat")
#define TITLE_ORIGINS ("Known Peer ID's")
#define TITLE_DIRECTCHAT ("Direct Messages")

#define TITLE_SHAREFILE ("Share File")
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
        void setDownloadInfo(QMap<QString,DownloadStatus::Status>*);
        void setSearchResults(QMultiHash< QString,QPair<QString,QByteArray> >*);
        void setGroupConvo(QList<Message>*);
        void setDirectStore(QMap<QString,QList<Message> >*);

    signals:
        void processNeighbor(Peer);
        void processFilesToShare(QStringList);
        void createChatRumor(QString);
        void createDirectChat(QString,QString);
        void refreshDirectConvo(QString);
        void searchForKeywords(QString);
        void requestFileFromPeer(QString,QPair<QString,QByteArray>);

    public slots:
        void gotRefreshGroupConvo();
        void gotRefreshDirectConvo(QString);
        void gotRefreshOrigins(QStringList);
        void gotRefreshNeighbors(QList<Peer>);
        void gotRefreshSharedFiles();
        void gotRefreshDownloadInfo();
        void gotRefreshSearchResults();

    private slots:
        void originSelected(QListWidgetItem*);
        void searchresultSelected(QListWidgetItem*);
        void gotGroupChatEntered();
        void gotDirectChatEntered();
        void gotNeighborEntered();
        void gotOpenFileDialog();
        void gotFileSearchEntered();

    private:
        Peerster* peerster;
        QMap<QString, quint32>* sharedFileInfo;
        QMap<QString, DownloadStatus::Status>* downloadInfo;
        QMultiHash< QString,QPair<QString,QByteArray> >* searchResults;
        QMap< QString,QList<Message> >* directStore;
        QList<Message>* groupConvo;
        QString ID;
        QGridLayout* mainlayout;
        QVBoxLayout *groupchatlayout, *neighborlayout, 
                    *directchatlayout, *originslayout, 
                    *filesharelayout, *filesearchlayout;
        QTextEdit *neighborview, *groupchatview, *directchatview, 
                  *fileshareview, *downloadsview;
        EntryQTextEdit *filesearchentry, *groupchatentry, 
                       *neighborentry, *directchatentry;
        QPushButton *addneighborbutton, *sharefilebutton;
        QListWidget *originslist, *searchresultlist;
        void createFileShareLayout();
        void createFileSearchLayout();
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