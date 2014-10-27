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
        File(QString,QString,QByteArray);
        ~File();
        QString name();
        QString abspath();
        quint32 size();
        bool isComplete();
        bool isShared();
        QByteArray ID();
        QByteArray metadata();
        QByteArray block(QByteArray);
        bool containsBlock(QByteArray); // block is a part of file
        bool hasBlock(QByteArray); // currently has actual block
        bool addBlockID(QByteArray);
        bool addBlock(QByteArray,QByteArray);
        bool addBlockID(quint32,QByteArray);
        void share();

    private:
        QString fileNameOnly, filePath, tempDirPath, 
                downloadsDirPath;
        quint32 fileSize, fileSizeMin, fileSizeMax;
        bool complete, shared;
        QFile *qfile, *metaFile;
        QByteArray* metaFileID;
        QList<QByteArray>* blockIDs; // ID in here != have block data
        QHash<QByteArray,QFile*>* blockTable;
        void assemble();
        void cleanupDownloads();
        QString metaFileTempPath();
        QString blockFileTempPath(quint32);
        QString blockFileDownloadsPath(quint32);
        QFile* writeByteArray(QString,QByteArray);
        QByteArray readNBytesFromStream(quint32,QDataStream*);
        QByteArray readBytesFromFile(QFile*);
        bool operator==(File);
        bool operator!=(File);
};

#endif // PEERSTER_FILE_HH