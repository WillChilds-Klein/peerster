#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#define POP_RATE (2000)             // 2s in ms
#define REAP_RATE (5000)            // 5s in ms
#define CYCLE_THRESHOLD (10)
#define BLOCK_REQUEST_LIMIT (10)
#define SHARED_FILE_DIR_PREFIX ("/tmp/peerster-")
#define TEMPDIR_NDIGITS (5)

#define BLOCK_HOP_LIMIT (10)
#define SEARCH_BUDGET_INIT (8)
#define SEARCH_BUDGET_LIMIT (16)
#define SEARCH_RESULTS_LIMIT (10)
#define BUDGET_INCREMENT (2)
#define BUDGET_INC_RATE (2000)      // 2s in ms
#define SEARCH_REPLY_HOP_LIMIT (10)

#include "Peerster.hh"

class Message;
class File;

class FileStore : public QObject
{
    Q_OBJECT

    class Search;
    class DownloadQueue;
    class Download;

    public:
        FileStore(Peerster*);
        ~FileStore();
        void setID(QString);
        void setSharedFileInfo(QMap<QString,quint32>*);
        void setDownloadInfo(QMap<QString,DownloadStatus::Status>*);
        void setSearchResults(QMultiHash< QString,QPair<QString,QByteArray> >*);

    signals:
        void postToInbox(Message,Peer);
        void refreshSharedFiles();
        void sendDirect(Message,QString);
        void updateDownloadInfo(Download);
        void refreshDownloadInfo();
        void refreshSearchResults();

    public slots:
        void gotProcessFilesToShare(QStringList);
        void gotSearchForKeywords(QString);
        void gotRequestFileFromPeer(QString,QPair<QString,QByteArray>);
        void gotProcessBlockRequest(Message);
        void gotProcessBlockReply(Message);
        void gotProcessSearchRequest(Message);
        void gotProcessSearchReply(Message);

    private slots:
        void gotPopChime();
        void gotReapChime();
        void gotUpdateDownloadInfo(Download);

    protected:
        void timerEvent(QTimerEvent*);

    private:
        Peerster* peerster;
        QString ID;
        QList<File>* sharedFiles;
        DownloadQueue *pendingDownloads;
        QMap<QString,quint32>* sharedFileInfo;
        QMap<QString,DownloadStatus::Status>* downloadInfo;
        QHash<int,Search*>* pendingSearches;
        QMultiHash<QString, QPair<QString,QByteArray> >* searchResults;
        QDir *tempdir, *downloads;
        QTimer *popTimer, *reapTimer;
        void makeTempdir();
        int searchIDByKeywords(QString);
        void killSearch(int);
        QStringList getSharedFileNames();
        bool enDequeuePendingDownloadQueue(); // true if head re-queue
        void cyclePendingDownloadQueue();
};

class FileStore::Search
{
    public:
        Search(Message*,int);
        ~Search();
        void incrementBudget();
        void incrementResults();
        int budget();
        int results();
        QString keywords();
        Message message();

    private:
        Message* msg;
        int nResults, searchID;
        QString keywordString;
};

class FileStore::Download : private QHash<QByteArray,quint32>
{
    public:
        Download();
        Download(File*,QString);
        ~Download();
        QString toString();
        File* fileObject();
        QString peer();
        DownloadStatus::Status status();
        QList<QByteArray> blocksNeeded();
        bool needsMetaData();
        bool needsBlock(QByteArray);
        bool isAlive();
        void begin();
        void touch(QByteArray);
        void addMetaData(QByteArray);
        void addBlockData(QByteArray,QByteArray);

    private:
        File* file;
        QString peerID;
        DownloadStatus::Status downloadStatus;
        void confirm();
        void complete();
        void kill();
};


class FileStore::DownloadQueue : private QList<Download*>
{
    public:
        DownloadQueue(FileStore*);
        ~DownloadQueue();
        quint32 size();
        Download* dequeue();
        void enqueue(Download*);
        Download* search(QByteArray); // ret ptr to val, NULL on not found.
        void reap();
        bool isEmpty();
        
    private:
        FileStore* filestore;
};

#endif // PEERSTER_FILESTORE_HH