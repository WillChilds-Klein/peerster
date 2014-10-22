#ifndef PEERSTER_FILE_HH
#define PEERSTER_FILE_HH

#include "Peerster.hh"

#define BLOCK_SIZE (8000)

#define FILE_ID_MAX (100000)

class File // : public QObject
{
    // Q_OBJECT

    public:
        File(QString,QString);
        File(QByteArray*);
         ~File();
        QString name();
        QString abspath();
        quint32 size();
        QString ID();
        QString blockFileName(quint32);

    private:
        QString fileName;
        QString absFilePath;
        quint32 fileSize;
        QString fileID;
        QString tempDirPath;
        QFile* qfile;
        QList<QFile*>* blocks;
        QList<QByteArray>* blockHashes;
        QFile* metaFile;
        QByteArray metaFileHash;
        void processFile();
        bool operator==(File);
        bool operator!=(File);
};

#endif // PEERSTER_FILE_HH