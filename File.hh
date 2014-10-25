#ifndef PEERSTER_FILE_HH
#define PEERSTER_FILE_HH

#include "Peerster.hh"

#define BLOCK_SIZE (8000)
#define HASH_SIZE (20)
#define FILE_ID_MAX (100000)
#define DOWNLOADS_DIR_NAME ("downloads")

class File // : public QObject
{
    // Q_OBJECT

    public:
        File(QString,QString);
        File(QString,QString, QByteArray*);
        ~File();
        QString name();
        QString abspath();
        quint32 size();
        QString ID();
        QString blockFileName(quint32);
        void share();

    private:
        QString fileName, absFilePath, fileID, 
                tempDirPath, downloadsDirPath;
        quint32 fileSize;
        bool complete, shared;
        QFile *qfile, *metaFile;
        QList<QFile*>* blocks;
        QList<QByteArray>* blockHashes;
        QByteArray* metaFileHash;
        void processFileForSharing();
        bool operator==(File);
        bool operator!=(File);
};

#endif // PEERSTER_FILE_HH