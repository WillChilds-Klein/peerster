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

    public:
        FileStore(Peerster*);
        ~FileStore();
        void setID(QString);
        void setSharedFileInfo(QMap<QString,quint32>*);

    signals:
        void refreshSharedFiles();
        void sendDirect(Message,QString);
        void recievedID(QByteArray);

    public slots:
        void gotProcessFilesToShare(QStringList);
        void gotRequestFile(QString,QString);
        // void gotProcessFileSearchRequest(QStringList);
        void gotProcessBlockRequest(Message);
        void gotProcessBlockReply(Message);
        void gotProcessSearchRequest(Message);
        void gotProcessSearchReply(Message);

    private slots:
        void gotBlockRequestChime();

    private:
        Peerster* peerster;
        QString ID;
        QList<File>* sharedFiles;
        QList< QPair<File,QString> >* pendingFileDownloads;
        QHash<QByteArray,quint32>* pendingRequests;
        QMap<QString, quint32>* sharedFileInfo;
        QMap<QString, quint32>* downloadInfo;
        QDir *tempdir, *downloads;
        QTimer* blockRequestTimer;
        void makeTempdir();
        bool maxRequestsMade(QByteArray);
};

#endif // PEERSTER_FILESTORE_HH