#ifndef PEERSTER_FILE_HH
#define PEERSTER_FILE_HH

#include "Peerster.hh"

#define BLOCK_SIZE ("8000")

#define ID_MAX (100000)

class File
{
    // Q_OBJECT

    public:
        File(QString);
        ~File();
        QString name();
        QString abspath();
        quint32 size();
        QString ID();

    private:
        QString fileName;
        QString absFilePath;
        quint32 fileSize;
        QString fileID;
        QString tempDirPath;
        QFile* qfile;
        QList<QFile>* blocks;
        QList<QFile>* blockHashes;
        QFile* metafile;
        QFile* metafileHash;
        void splitFile();
        void hashBlocks();
        void generateMetafile();
        void hashMetafile();
        bool operator==(Peer);
        bool operator!=(Peer);
};

#endif // PEERSTER_FILE_HH