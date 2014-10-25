#include "File.hh"

File::File(QString absolutepath, QString temppath)
    : qfile(new QFile)
    , complete(false)
    , shared(false)
    , metaFile(new QFile)
    , blocks(new QList<QFile*>)
    , blockIDs(new QList<QByteArray>)
    , blockIDTable(new QHash<QByteArray,QFile*>)
{
    if(absolutepath.isEmpty())
    {
        qDebug() << "FILE CONSTRUCTED FROM EMPTY PATH!";
        return;
    }

    qfile->setFileName(absolutepath);
    if (!qfile->open(QIODevice::ReadOnly)) {
        qDebug() << "FILE " << absolutepath << "DOESN'T EXIST!";
        return;
    }

    fileName = absolutepath.split("/").last();
    absFilePath = qfile->fileName();
    tempDirPath = temppath + 
                 (temppath.endsWith("/") ? "" : "/");
    downloadsDirPath = QDir::currentPath() + DOWNLOADS_DIR_NAME + "/";

    fileSize = qfile->size();

    complete = true;

    share();

    qDebug() << "CREATED FILE " << fileName << " (" 
             << fileSize << " B)";
    qDebug() << "LOCATED AT " << absFilePath;
    qDebug() << "TEMP DIR:" << tempDirPath;
}

File::File(QString fn, QString temppath, QByteArray* metaHashBits)
    : qfile(new QFile)
    , fileName(fn)
    , complete(false)
    , shared(false)
    , metaFile(new QFile)
    , blocks(new QList<QFile*>)
    , blockIDs(new QList<QByteArray>)
    , blockIDTable(new QHash<QByteArray,QFile*>)
{
    if(metaHashBits == NULL || metaHashBits->size() != HASH_SIZE)
    {
        qDebug() << "INVALID HASH!";
        return;
    }

    downloadsDirPath = QDir::currentPath() + DOWNLOADS_DIR_NAME + "/";

    fileName = fn;
    absFilePath = downloadsDirPath + fn;
    tempDirPath = temppath + 
                 (temppath.endsWith("/") ? "" : "/");

    metaFileID = new QByteArray(*metaHashBits);

    qDebug() << "DOWNLOADED FILE " << fileName;
    qDebug() << "LOCATED AT " << absFilePath;
    qDebug() << "TEMP DIR:" << tempDirPath;
}

File::~File()
{
    // qfile->close();
    // delete(qfile);
    // qDeleteAll(blocks);
    // delete(blocks);
    // delete(blockIDs);
    // delete(metaFile);
}

QString File::name()
{
    return fileName;
}

QString File::abspath()
{
    return absFilePath;
}

quint32 File::size()
{
    return fileSize;
}

QByteArray File::ID()
{
    return *metaFileID;
}

QByteArray File::metafile()
{
    QByteArray fileData;
    if(metaFile != NULL && metaFile->open(QIODevice::ReadOnly))
    {
        QDataStream fileDataStream(metaFile);
        fileData << fileDataStream;
        metaFile->close();
    }
    else
    {
        qDebug() << "COULDN'T LOCATE METAFILE FOR" << fileName;
    }

    return fileData;
}

QByteArray File::block(QByteArray blockID)
{
    QByteArray blockData;
    QFile blockFile = blockIDTable[blockID];
    if(blockIDTable->contains(blockID) && 
        blockFile->open(QIODevice::ReadOnly))
    {
        QDataStream blockDataStream(blockFile);
        blockData << blockDataStream;
        blockFile->close();
    }
    else
    {
        qDebug() << "COULDN'T LOCATE BLOCKFILE " << blockID;
    }

    return blockData;
}

bool File::containsBlock(QByteArray blockID)
{
    return blockIDTable->contains(blockID);
}

void File::share()
{
    processFileForSharing();

    shared = true;

    qDebug() << "SHARED FILE " << fileName << " (" 
             << fileSize << " B)";
    qDebug() << "LOCATED AT " << absFilePath;
    qDebug() << "TEMP DIR:" << tempDirPath;
}

QString File::blockFileName(quint32 i)
{
    return tempDirPath + fileName + "." + QString::number(i) + ".block";
}

void File::processFileForSharing()
{
    QCA::Hash sha("sha1");
    QDataStream fileInStream(qfile);
    QString blockName;
    QByteArray blockData, blockID, metaData;
    QFile* blockFile;
    char buffer[BLOCK_SIZE];
    int readSize, i;

    i = 1;
    while(!fileInStream.atEnd())
    {
        readSize = fileInStream.readRawData(buffer, BLOCK_SIZE);
        blockData.setRawData(buffer, readSize);
        blockName = blockFileName(i);
        qDebug() << "BLOCKFILE " << i << "NAME: " << blockName;
        blockFile = new QFile(blockName);

        // write block file.
        if (blockFile->open(QIODevice::WriteOnly | QIODevice::ReadOnly))
        {
            QDataStream blockOutStream(blockFile);
            blockOutStream.writeRawData(buffer, readSize);
            blockFile->close();
        }
        else
        {
            qCritical() << "COULDN'T CREATE BLOCK " << i << " FOR " 
                                                    << fileName << "!";
            return;
        }
        blocks->append(blockFile);

        sha.update(blockFile);
        blockID = sha.final().toByteArray();
        blockIDs->append(blockID);
        blockIDTable[blockID] = blockFile;

        // update metaFile
        metaData.append(blockID);

        blockData.clear();
        blockID.clear();
        sha.clear();
        i++;
    }

    // write metaFile
    metaFile->setFileName(tempDirPath + fileName + ".meta");
    if(metaFile->open(QIODevice::WriteOnly | QIODevice::ReadOnly))
    {
        QDataStream metaOutStream(metaFile);
        metaOutStream << metaData;
    }
    else
    {
        qCritical() << "COULDN'T CREATE METAFILE FOR "<< fileName << "!";
    }

    // hash metaFile
    sha.update(metaFile);
    metaFileID = new QByteArray(sha.final().toByteArray());
    metaFile->close();

    qDebug() << "HEX METAFILE HASH: " << QCA::arrayToHex(*metaFileID);
}

bool File::operator==(File other)
{
    return (this->ID() == other.ID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}



