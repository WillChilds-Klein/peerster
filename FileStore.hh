#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#include "Peerster.hh"

#define TEMPDIR_NDIGITS ("5")
#define BLOCK_SIZE ("8000")

class Message;

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

    public slots:
        void gotProcessFilesToShare(QStringList);

    private:
        Peerster* peerster;
        QString ID;
        QList<File>* sharedFiles;
        QMap<QString, quint32>* sharedFileInfo;
        QDir* tempdir;
};

#endif // PEERSTER_FILESTORE_HH