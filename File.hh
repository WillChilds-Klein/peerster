#ifndef PEERSTER_FILE_HH
#define PEERSTER_FILE_HH

#define BLOCK_SIZE (8000)
#define HASH_SIZE (20)
#define FILE_ID_MAX (100000)
#define DOWNLOADS_DIR_NAME ("downloads")

#define METAFILE_APPENDAGE (".meta")
#define BLOCKFILE_APPENDAGE (".block")

#include "Peerster.hh"

class File 
{
    public:
        File(QString,QString);
        File(QString,QString,QByteArray);
        ~File();
        QString toString();
        QString name();
        QString abspath();
        quint32 size();
        QByteArray fileID();
        QByteArray metadata();
        QByteArray block(QByteArray);
        QList<QByteArray> blockIDs();
        bool isComplete();
        bool isShared();
        bool containsBlock(QByteArray);     // block is a part of file
        bool hasMetaData();
        bool hasBlock(QByteArray);  // currently has actual block data
        bool addMetaData(QByteArray);        // true on successful add
        bool addBlock(QByteArray,QByteArray);// true on successful add
        void share();
        static quint32 writeByteArrayToFile(QFile*,QByteArray);
        static QByteArray readNBytesFromStream(quint32,QDataStream*);
        static QByteArray readBytesFromFile(QFile*);

    private:
        QString fileNameOnly, filePath, tempDirPath, 
                downloadsDirPath;
        quint32 fileSize;
        bool complete, shared;
        QFile *qfile, *metaFile;
        QByteArray metaFileID;
        QList<QByteArray>* blockIDList; // ID in here != have block data
        QHash<QByteArray,QFile*>* blockTable;
        void assemble();
        void cleanup();
        QString metaFileName();
        QString blockFileName(QByteArray,quint32);
        bool operator==(File);
        bool operator!=(File);
};

#endif // PEERSTER_FILE_HH