#include "File.hh"

File::File(QString absolutepath, QString temppath)
    : tempDirPath(temppath)
    , filePath(absolutepath)
    , complete(false)
    , shared(false)
    , qfile(new QFile)
    , metaFile(new QFile)
    , metaFileID(new QByteArray())
    , blockIDs(new QList<QByteArray>)
    , blockTable(new QHash<QByteArray,QFile*>)
{
    if(absolutepath.isEmpty())
    {
        qDebug() << "FILE CONSTRUCTED FROM EMPTY PATH!";
        return;
    }

    qfile->setFileName(absolutepath);
    if (!qfile->exists()) {
        qDebug() << "FILE " << absolutepath << "DOESN'T EXIST!";
        return;
    }

    fileNameOnly = absolutepath.split("/").last();
    filePath = absolutepath;
    tempDirPath = temppath + 
                 (temppath.endsWith("/") ? "" : "/");
    downloadsDirPath = QDir::currentPath() + 
                      (QDir::currentPath().endsWith("/") ? "" : "/") +
                       DOWNLOADS_DIR_NAME + "/";

    fileSize = fileSizeMin = fileSizeMax = qfile->size();

    share();
}

File::File(QString absolutepath, QString temppath, QByteArray metaFileIDBytes)
    : tempDirPath(temppath)
    , filePath(absolutepath)
    , complete(false)
    , shared(false)
    , qfile(new QFile)
    , metaFile(new QFile)
    , metaFileID(new QByteArray(metaFileIDBytes))
    , blockIDs(new QList<QByteArray>)
    , blockTable(new QHash<QByteArray,QFile*>)
{
    if(metaFileID.size() == 0 || metaFileID->size() != HASH_SIZE)
    {
        qDebug() << "INVALID FILEID PASSED TO "
                 << "File(QString,QString,QByteArray) CONSTRUCTOR!";
        return;
    }

    fileNameOnly = absolutepath.split("/").last();
    tempDirPath += (temppath.endsWith("/") ? "" : "/");
    downloadsDirPath = QDir::currentPath() + 
                      (QDir::currentPath().endsWith("/") ? "" : "/") +
                       DOWNLOADS_DIR_NAME + "/";

    qDebug() << "DOWNLOADING FILE " << fileNameOnly;
    qDebug() << "LOCATED AT " << filePath;
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
    return fileNameOnly;
}

QString File::abspath()
{
    return filePath;
}

quint32 File::size()
{
    return fileSize;
}

bool File::isComplete()
{
    return complete;
}

bool File::isShared()
{
    return shared;
}

QByteArray File::ID()
{
    return *metaFileID;
}

QByteArray File::metadata()
{
    return readBytesFromFile(metaFile);
}

QByteArray File::block(QByteArray blockID)
{
    QByteArray blockData;
    QFile* blockFile; 
    if(blockTable->contains(blockID))
    {
        blockFile = (*blockTable)[blockID];
        blockData = readBytesFromFile(blockFile);
    }
    else
    {
        qDebug() << "UNKOWN BLOCKID: " << blockID;
    }

    return blockData;
}

bool File::containsBlock(QByteArray blockID)
{
    return blockIDs->contains(blockID);
}

bool File::hasBlock(QByteArray blockID)
{
    return blockTable->contains(blockID);
}

bool File::addBlockID(quint32 index, QByteArray blockID)
{
    if(!complete && !blockIDs->contains(blockID) && 
        index >= 0 && index <= blockIDs->size())
    {
        blockIDs->insert(index, blockID);
    }

    if(blockIDs->size() == blockTable->size())
    {
        assemble();
        cleanupDownloads();
        complete = true;
    }
}

bool File::addBlock(QByteArray blockID, QByteArray blockData)
{
    if(!complete && !blockIDs->contains(blockID))
    {
        (*blockTable)[blockID] = blockData;
        writeByteArray(blockFileDownloadsPath(blockIDs->indexOf(blockID)), blockData);
        
    }

    return false;
}

void File::share()
{
    QCA::Hash sha("sha1");
    int readSize, i;
    char buffer[BLOCK_SIZE];    
    QString blockFilePath;
    QFile* blockFile;
    QByteArray blockData, 
               blockID, 
               metaData;

    if(!qfile->open(QIODevice::ReadOnly))
    {
        qDebug() << "COULDN'T READ FILE: " << f->fileName();
        return;
    }
    QDataStream in(qfile);

    // TODO: Make this code not shitty...
    i = 1;
    while(!in.atEnd())
    {
        readSize = in.readRawData(buffer, BLOCK_SIZE);
        blockData.setRawData(buffer, readSize);
        blockFilePath = blockFileTempPath(i);
        qDebug() << "BLOCKFILE " << i << "NAME: " << blockFilePath;
        blockFile = new QFile(blockFilePath);

        // write block file...move over writeByteArray method..
        if (blockFile->open(QIODevice::WriteOnly | QIODevice::ReadOnly))
        {
            QDataStream blockOutStream(blockFile);
            blockOutStream.writeRawData(buffer, readSize);
            blockFile->close();
        }
        else
        {
            qCritical() << "COULDN'T CREATE BLOCK " << i << " FOR " 
                                                    << fileNameOnly << "!";
            return;
        }
        sha.update(blockFile);
        blockID = sha.final().toByteArray();
        blockIDs->append(blockID);
        (*blockTable)[blockID] = blockFile;

        // update metaFile
        metaData.append(blockID);

        blockData.clear();
        blockID.clear();
        sha.clear();
        i++;
    }

    // write metaFile...move over to writeByteArray method...
    metaFile->setFileName(metaFilePath());      //          |
    // metaFile = writeByteArray(metaFilemetaFile);      <---
    if(metaFile->open(QIODevice::WriteOnly | QIODevice::ReadOnly))
    {
        QDataStream metaOutStream(metaFile);
        metaOutStream << metaData;
    }
    else
    {
        qCritical() << "COULDN'T CREATE METAFILE FOR "<< fileNameOnly << "!";
    }

    // generate metaFileID
    sha.update(metaFile);
    metaFile->close();
    metaFileID = new QByteArray(sha.final().toByteArray());

    shared = true;

    qDebug() << "SHARING FILE " << fileNameOnly << " (" << fileSize << " B)";
    qDebug() << "LOCATED AT " << filePath;
    qDebug() << "TEMP DIR:" << tempDirPath;
    qDebug() << "METAFILE ID (HEX): " 
             << QCA::arrayToHex(*metaFileID) << "\n";
}

void File::assemble()
{
    QByteArray filebytes;

    foreach(QByteArray blockID, *blockIDs)
    {
        filebytes.append(readBytesFromFile(blockTable[blockID]));
    }

    writeByteArray(filePath, filebytes);
}

void File::cleanupDownloads()
{
    foreach(QByteArray blockID, blockTable->keys())
    {
        if(!blockTable[blockID]->remove())
        {
            qDebug() << "ERROR REMOVING FILE " 
                     << blockTable[blockID]->fileName();
        }
    }
}

QString File::metaFileTempPath()
{
    return tempDirPath + fileNameOnly + ".meta";
}

QString File::blockFileTempPath(quint32 i)
{
    return tempDirPath + fileNameOnly + "." + QString::number(i) + ".block";
}

QString File::blockFileDownloadsPath(quint32 i)
{
    return downloadsDirPath + fileNameOnly + "." + 
                              QString::number(i) + ".block";
}

QFile* File::writeByteArray(QString filepath, QByteArray bytes)
{
    QFile writeFile = new QFile(filepath);
    if (writeFile->open(QIODevice::WriteOnly))// | QIODevice::ReadOnly))
    {
        QDataStream outStream(writeFile);
        outStream << bytes;
        writeFile->close();
    }
    else
    {
        qCritical() << "COULDN'T WRITE FILE: "  << filepath;
    }

    return writeFile;
}

QByteArray File::readNBytesFromStream(quint32 n, QDataStream* in)
{
    // TODO
}

QByteArray File::readBytesFromFile(QFile* f)
{
    QByteArray bytes;
    if(f == NULL)
    {
        qDebug() << "NULL FILE PTR PASSED TO " 
                 << "readBytesFromFile(QFile*)!";
    }
    else if(f->open(QIODevice::ReadOnly))
    {
        QDataStream inStream(f);
        inStream >> bytes;
        f->close();
    }
    else
    {
        qDebug() << "COULDN'T READ FILE: " << f->fileName();
    }

    return bytes;
}

bool File::operator==(File other)
{
    return (this->ID() == other.ID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}



