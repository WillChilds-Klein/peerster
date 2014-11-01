#include "File.hh"

File::File(QString absolutepath, QString temppath)
    : tempDirPath(temppath)
    , filePath(absolutepath)
    , complete(false)
    , shared(false)
    , qfile(new QFile)
    , metaFile(NULL)
    , metaFileID(QByteArray())
    , blockIDList(new QList<QByteArray>)
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
    fileSize = qfile->size();

    complete = true;

    qDebug() << "fileNameOnly: " << fileNameOnly;
    qDebug() << "filePath: " << filePath;
    qDebug() << "tempDirPath: " << tempDirPath;
    qDebug() << "downloadsDirPath: " << downloadsDirPath;
    qDebug() << "fileSize: " << QString::number(fileSize);
}

File::File(QString absolutepath, QString temppath, QByteArray metaFileIDBytes)
    : tempDirPath(temppath)
    , filePath(absolutepath)
    , complete(false)
    , shared(false)
    , qfile(NULL)            // potential segfault?
    , metaFile(NULL)         // potential segfault?
    , metaFileID(QByteArray(metaFileIDBytes))
    , blockIDList(new QList<QByteArray>)
    , blockTable(new QHash<QByteArray,QFile*>)
{
    if(metaFileID.size() == 0 || (metaFileID.size() != HASH_SIZE))
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
    // TODO: uncomment this shniz...
    // qfile->close();
    // delete(qfile);
    // qDeleteAll(blocks);
    // delete(blocks);
    // delete(blockIDList);
    // delete(metaFile);
}

QString File::toString()
{
    QString qstr = "===== File " + fileNameOnly + " (" + 
            QString::number(fileSize) + " B) =====\n";
    qstr += "fileID: " + QString(metaFileID.toHex()) + "\n";
    qstr += "metadata: " + (metaFile == NULL
                            ? "METADATA HASN'T BEEN SET YET!!" 
                            : QString(metadata().toHex()))
                         + "\n";
    
    qstr += "blockIDs (size " + QString::number(blockIDList->size()) 
          + "): [ ";
    foreach(QByteArray blockID, *blockIDList)
    {
        qstr += "{" + QString(blockID.toHex()) + "},";
    }
    qstr += " ]\n";

    qstr += "have blocks: [";
    if(blockTable->isEmpty())
    {
        qstr += " NO BLOCKS YET ";
    }
    foreach(QByteArray blockID, blockTable->keys())
    {
        qstr += "{" + QString(blockID.toHex()) + "},";
    }
    qstr += "]\n";
    qstr += "==============================\n";

    return qstr;
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

QByteArray File::fileID()
{
    return metaFileID;
}

QByteArray File::metadata()
{
    return metaFile != NULL ? readBytesFromFile(metaFile) : QByteArray();
}

QByteArray File::block(QByteArray blockID)
{
    QByteArray blockData;

    if(blockTable->empty())
    {
        qDebug() << "DON'T HAVE ANY BLOCK DATA YET!";
    }
    else if(blockTable->contains(blockID))
    {
        QFile* blockFile = (blockTable->value(blockID));
        blockData = readBytesFromFile(blockFile);
    }
    else
    {
        qDebug() << "UNKOWN BLOCKID: " << blockID;
    }

    return blockData;
}

QList<QByteArray> File::blockIDs()
{
    return *blockIDList;
}

bool File::containsBlock(QByteArray blockID)
{
    return blockIDList->contains(blockID);
}

bool File::hasMetaData()
{
    return metaFile != NULL && !blockIDList->empty();
}

bool File::hasBlock(QByteArray blockID)
{
    return blockTable->contains(blockID);
}

bool File::addMetaData(QByteArray metaFileBytes)
{
    if(metaFile != NULL || !blockIDList->empty())
    {
        return false;
    }

    // write metaFile
    metaFile = new QFile(tempDirPath + metaFileName());
    writeByteArrayToFile(metaFile, metaFileBytes);

    QByteArray blockBytes;
    qDebug() << "BLOCK IDS PARSED FROM METADATA:";
    for(int i = 0; i < metaFileBytes.size(); i += HASH_SIZE)
    {
        blockBytes = metaFileBytes.mid(i, HASH_SIZE);
        blockIDList->append(blockBytes);
        qDebug() << "\t" << QString(blockBytes.toHex());
    }

    return true;
}

bool File::addBlock(QByteArray blockID, QByteArray blockData)
{
    if(complete || !blockIDList->contains(blockID))
    {
        return false;
    }

    // write blockData to file
    QString blockFilePath = downloadsDirPath + 
                            blockFileName(blockID, 
                                          blockIDList->indexOf(blockID));
    QFile* blockFile = new QFile(blockFilePath);
    writeByteArrayToFile(blockFile, blockData);
    
    // update blockTable
    blockTable->insert(blockID, blockFile);

    // check to see if we've fully populated file
    if(blockTable->size() == blockIDList->size())
    {
        qDebug() << "HAVE ALL BLOCKS FOR FILE " 
                 << QString(metaFileID.toHex());

        assemble();
        cleanup();

        complete = true;
    }

    return true;
}

void File::share()
{
    int readSize, writeSize, i;
    char buffer[BLOCK_SIZE];    
    QString blockFilePath;
    QFile* blockFile;
    QByteArray blockData, 
               blockID, 
               metaData;

    if(qfile == NULL)
    {
        qDebug() << "TRYING TO SHARE FILE THAT DOESN'T EXIST!!";
        return;
    }
    else if(!qfile->open(QIODevice::ReadOnly))
    {
        qDebug() << "COULDN'T OPEN FILE: " << qfile->fileName();
        return;
    }

    QDataStream* in = new QDataStream(qfile);

    i = 1;
    while(!in->atEnd())
    {
        blockData = readNBytesFromStream(BLOCK_SIZE, in);
        readSize = blockData.size();
        if(readSize <= 0)
        {
            qCritical() << "COULDN'T READ BLOCK " << i << " FROM FILE "
                                                  << fileNameOnly << "!";
        }

        // compute blockID
        QCA::Hash sha("sha1");
        sha.update(blockData);
        blockID = sha.final().toByteArray();

        // write blockfile
        blockFile = new QFile(tempDirPath + blockFileName(blockID, i));
        writeSize = writeByteArrayToFile(blockFile, blockData);
        if(writeSize <= 0)
        {
            qCritical() << "COULDN'T WRITE BLOCKFILE " << i << " FOR " 
                                                       << fileNameOnly << "!";
            return;
        }

        // update blockIDList
        blockIDList->append(blockID);

        // update blockTable
        blockTable->insert(blockID, blockFile);

        // update metaFile
        metaData.append(blockID);

        blockData.clear();
        blockID.clear();
        i++;
    }
    delete(in);

    // compute metaFileID
    QCA::Hash metaSha("sha1");
    metaSha.update(metaData);
    metaFileID = QByteArray(metaSha.final().toByteArray());

    // write metaFile
    metaFile = new QFile(tempDirPath + metaFileName());
    writeByteArrayToFile(metaFile, metaData);

    shared = true;


    qDebug() << "SHARED FILE " << fileNameOnly << " (" << fileSize << " B)";
    qDebug() << "LOCATED AT " << filePath;
    qDebug() << "TEMP DIR:" << tempDirPath;
    qDebug() << "FINAL METADATA:" << metaData.toHex();
    qDebug() << "METAFILE ID (HEX): " 
             << QCA::arrayToHex(metaFileID) << "\n";
}

quint32 File::writeByteArrayToFile(QFile* file, QByteArray bytes)
{
    quint32 bytesWritten = -1;

    if(file == NULL)
    {
        qDebug() << "NULL QFILE PTR PASSED TO " 
                 << "readNBytesFromStream(quint32,QDataStream)!";
    }
    else if (file->open(QIODevice::WriteOnly))
    {
        bytesWritten = file->write(bytes);
        file->close();
    }
    else
    {
        qCritical() << "COULDN'T WRITE FILE: "  << file->fileName();
    }

    return bytesWritten;
}

QByteArray File::readNBytesFromStream(quint32 n, QDataStream* in)
{
    int readSize;
    char rawBytes[n];
    QByteArray bytes;

    if(in == NULL)
    {
        qDebug() << "NULL QDATASTREAM PTR PASSED TO " 
                 << "readNBytesFromStream(quint32,QDataStream)!";
    }
    else if(!in->atEnd())
    {
        readSize = in->readRawData(rawBytes, n);
        if(readSize <= 0)
        {
            qDebug() << "ERROR READING FROM QDATASTREAM!!";
            return bytes;
        }

        bytes = QByteArray(rawBytes, readSize);
    }
    else
    {
        qDebug() << "QDATASTREAM ALREADY AT EOF!";
    }

    return bytes;
}

QByteArray File::readBytesFromFile(QFile* f)
{
    QByteArray bytes;

    if(f == NULL)
    {
        qDebug() << "NULL QFILE PTR PASSED TO " 
                 << "readBytesFromFile(QFile*)!";
    }
    else if(f->open(QIODevice::ReadOnly))
    {
        bytes = f->readAll();

        qDebug() << "JUST READ FROM FILE " << f->fileName() 
                 << ": " << QString::number(bytes.size())  << " bytes";
        f->close();
    }
    else
    {
        qDebug() << "COULDN'T READ FILE: " << f->fileName();
    }

    return bytes;
}

void File::assemble()
{
    qDebug() << "ASSEMBLING FILE WITH ID " << QString(metaFileID.toHex())
             << "IN DIRECTORY " << downloadsDirPath;

    QByteArray filebytes;

    QList<QByteArray>::iterator i;
    for(i = blockIDList->begin(); i != blockIDList->end(); ++i)
    {
        QByteArray blockID = *i;
        filebytes.append(readBytesFromFile(blockTable->value(blockID)));
    }

    qfile = new QFile(filePath);
    writeByteArrayToFile(qfile, filebytes);

    qDebug() << "FINISHED DOWNLOADING " << qfile->fileName() 
             << " WHICH CAN BE FOUND AT " << filePath;
}

void File::cleanup()
{
    qDebug() << "CLEANING UP DOWNLOADS DIR FOR FILE WITH ID " 
           << QString(metaFileID.toHex());

    QHash<QByteArray,QFile*>::iterator i = blockTable->begin();
    while(i != blockTable->end())
    {
        if(i.value()->remove()) // rm blockfile from disk
        {
            delete(i.value());
            i = blockTable->erase(i);
        }
        else
        {
            qDebug() << "ERROR REMOVING BLOCKFILE " 
                     << i.value()->fileName();
            i++;
        }
    }

    if(metaFile->remove()) // rm metafile from disk.
    {
        delete(metaFile);
        metaFile = NULL;
    }
    else
    {
        qDebug() << "ERROR REMOVING METAFILE " 
                 << metaFile->fileName();
    }
}

QString File::metaFileName()
{
    return fileNameOnly + "-" + QString(metaFileID.toHex()) + METAFILE_APPENDAGE;
}

QString File::blockFileName(QByteArray blockID, quint32 i)
{
    return fileNameOnly + "-" + QString(blockID.toHex()) + "." + QString::number(i) + BLOCKFILE_APPENDAGE;
}

bool File::operator==(File other)
{
    return (fileID().size() == HASH_SIZE) && 
           (this->fileID() == other.fileID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}

