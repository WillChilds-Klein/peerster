#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#include "Peerster.hh"

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

    public slots:
        void gotProcessFilesToShare(QStringList);
        void gotRequestFile(QString,QString);
        // void gotProcessFileSearchRequest(QStringList);
        void gotProcessBlockRequest(Message);
        void gotProcessBlockReply(Message);

    private:
        Peerster* peerster;
        QString ID;
        QList<File>* sharedFiles;
        QMap<QString, quint32>* sharedFileInfo;
        QDir* tempdir;
        void makeTempdir();
};

#endif // PEERSTER_FILESTORE_HH