#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#include "Peerster.hh"

#define BLOCK_REQUEST_RATE (30000) // 30s in ms
#define BLOCK_REQUEST_LIMIT (5)
#define SHARED_FILE_DIR_PREFIX ("/tmp/peerster-")
#define TEMPDIR_NDIGITS (5)   

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
        DownloadQueue* pendingDownloads;
        QMap<QString, quint32>* sharedFileInfo;
        QMap<QString, quint32>* downloadInfo;
        QDir *tempdir, *downloads;
        QTimer* blockRequestTimer;
        void makeTempdir();
        bool maxRequestsMade(QByteArray);
};

class FileStore::Download : private QMap<QByteArray,quint32>
{
    public:
        Download(File*,QString);
        ~Download();
        File* fileObject();
        QString peer();
        QList<QByteArray> blocksNeeded();
        bool isAlive();
        void touch(QByteArray);
        void addBlockID(QByteArray);
        void removeBlockID(QByteArray);
        void addBlockToFile(QByteArray,QByteArray);
        void kill();

    private:
        File* file;
        QString peerID;
        bool alive;
};


class FileStore::DownloadQueue : private QList<Download>
{
    public:
        DownloadQueue(FileStore*);
        ~DownloadQueue();
        Download dequeue();
        void enqueue(Download);
        void processBlock(QByteArray,QByteArray);
        void cycle();
        void reap();
        
    private:
        FileStore* filestore;
};
#endif // PEERSTER_FILESTORE_HH