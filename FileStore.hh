#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#define BLOCK_REQUEST_RATE (30000) // 30s in ms
#define BLOCK_REQUEST_LIMIT (10)
#define SHARED_FILE_DIR_PREFIX ("/tmp/peerster-")
#define TEMPDIR_NDIGITS (5)

#include "Peerster.hh"

class Message;
class File;

class FileStore : public QObject
{
    Q_OBJECT

    class DownloadQueue;
    class Download;

    public:
        FileStore(Peerster*);
        ~FileStore();
        void setID(QString);
        void setSharedFileInfo(QMap<QString,quint32>*);
        void setDownloadInfo(QMap<QString,DownloadStatus>*);

    signals:
        void refreshSharedFiles();
        void sendDirect(Message,QString);
        void updateDownloadInfo(QString,DownloadStatus);
        void refreshDownloadInfo();

    public slots:
        void gotProcessFilesToShare(QStringList);
        void gotRequestFileFromPeer(QString,QString);
        // void gotProcessFileSearchRequest(QStringList);
        void gotProcessBlockRequest(Message);
        void gotProcessBlockReply(Message);
        void gotProcessSearchRequest(Message);
        void gotProcessSearchReply(Message);

    private slots:
        void gotBlockRequestChime();
        void gotUpdateDownloadInfo(QString,DownloadStatus);

    private:
        Peerster* peerster;
        QString ID;
        QList<File>* sharedFiles;
        DownloadQueue *confirmingDownloads, *pendingDownloads,
                      *completedDownloads, *failedDownloads;
        QMap<QString,quint32>* sharedFileInfo;
        QMap<QString,DownloadStatus>* downloadInfo;
        QDir *tempdir, *downloads;
        QTimer* blockRequestTimer;
        void makeTempdir();
        bool enDequeuePendingDownloadQueue(); // true if head re-queue
        void cyclePendingDownloadQueue();
};

class FileStore::Download : private QMap<QByteArray,quint32>
{
    typedef enum DownloadStatus
    {
        INIT        = 0
        CONFIRMING  = 1,
        PENDING     = 2,
        COMPLETE    = 3,
        FAILED      = 4,
        NONE        = 5
    }   DownloadStatus;
    
    public:
        Download();
        Download(File*,QString);
        ~Download();
        File* fileObject();
        QString peer();
        DownloadStatus status();
        QList<QByteArray> blocksNeeded();
        bool needs(QByteArray);
        bool needsMetaData();
        bool isAlive();
        void touch(QByteArray);
        void addMetaData(QByteArray);
        void addBlockData(QByteArray,QByteArray);

    private:
        File* file;
        QString peerID;
        DownloadStatus downloadStatus;
        void begin();
        void kill();
        void complete();
};


class FileStore::DownloadQueue : private QList<Download>
{
    public:
        DownloadQueue(FileStore*);
        ~DownloadQueue();
        quint32 size();
        Download dequeue();
        void enqueue(Download);
        Download* search(QByteArray); // ret ptr to val, NULL on not found.
        void reap();
        bool isEmpty();
        
    private:
        FileStore* filestore;
};
#endif // PEERSTER_FILESTORE_HH