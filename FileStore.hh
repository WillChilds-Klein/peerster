#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#define CYCLE_RATE (30000) // 30s in ms
#define REAP_RATE (5000)           // 5s in ms
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
        void setDownloadInfo(QMap<QString,DownloadStatus::Status>*);

    signals:
        void refreshSharedFiles();
        void sendDirect(Message,QString);
        void updateDownloadInfo(QString,DownloadStatus::Status);
        void refreshDownloadInfo();

    public slots:
        void gotProcessFilesToShare(QStringList);
        void gotRequestFileFromPeer(QString,QString);
        void gotProcessBlockRequest(Message);
        void gotProcessBlockReply(Message);
        void gotProcessSearchRequest(Message);
        void gotProcessSearchReply(Message);

    private slots:
        void gotCycleChime();
        void gotUpdateDownloadInfo(QString,DownloadStatus::Status);
        void gotReapChime();

    private:
        Peerster* peerster;
        QString ID;
        QList<File>* sharedFiles;
        DownloadQueue *pendingDownloads;
        QMap<QString,quint32>* sharedFileInfo;
        QMap<QString,DownloadStatus::Status>* downloadInfo;
        QDir *tempdir, *downloads;
        QTimer *cycleTimer, *reapTimer;
        void makeTempdir();
        bool enDequeuePendingDownloadQueue(); // true if head re-queue
        void cyclePendingDownloadQueue();
};

class FileStore::Download : private QMap<QByteArray,quint32>
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
        void touch(QByteArray);
        void addMetaData(QByteArray);
        void addBlockData(QByteArray,QByteArray);

    private:
        File* file;
        QString peerID;
        DownloadStatus::Status downloadStatus;
        void begin();
        void confirm();
        void complete();
        void kill();
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
        void upadateDownloadInfo(QString,DownloadStatus::Status);
        void reap();
        bool isEmpty();
        
    private:
        FileStore* filestore;
};
#endif // PEERSTER_FILESTORE_HH