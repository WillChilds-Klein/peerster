#include "File.hh"

File::File(QString absolutepath, QString temppath)
    : tempDirPath(temppath)
    , filePath(absolutepath)
    , complete(false)
    , shared(false)
    , qfile(new QFile)
    , metaFile(new QFile)
    , metaFileID(new QByteArray())
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
    , qfile(new QFile)
    , metaFile(new QFile)
    , metaFileID(new QByteArray(metaFileIDBytes))
    , blockIDList(new QList<QByteArray>)
    , blockTable(new QHash<QByteArray,QFile*>)
{
    if(metaFileID->size() == 0 || metaFileID->size() != HASH_SIZE)
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
    // delete(blockIDList);
    // delete(metaFile);
}

QString File::toString()
{
    QList<QByteArray>::iterator i;

    QString qstr = "\n===== File " + filePath + " (" + 
            QString::number(fileSize) + "KB) =====\n";
    qstr += "\tfileID: " + QString(metaFileID->toHex());
    qstr += "\tmetadata: " + QString(readBytesFromFile(metaFile).toHex());
    qstr += "\tblockIDs: [ ";
    for(i = blockIDList->begin(); i != blockIDList->end(); ++i)
    {
        qstr += "{" + QString(i->toHex()) + "},";
    }
    qstr += " ]\n";
    qstr += "\thave blocks: [ ";
    for(i = blockTable->keys().begin(); i != blockTable->keys().end(); ++i)
    {
        qstr += "{" + QString(i->toHex()) + "},";
    }
    qstr += " ]\n";
    qstr += "==========================";

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
    return !blockIDList->empty();
}

bool File::hasBlock(QByteArray blockID)
{
    return blockTable->contains(blockID);
}

bool File::addMetaData(QByteArray metaFileBytes)
{
    if(!blockIDList->empty())
    {
        return false;
    }

    QByteArray blockBytes;

    qDebug() << "METADATA: " << QString(metaFileBytes.toHex());
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
    if(!complete && blockIDList->contains(blockID))
    {
        // TODO: determine whether to use fileName.i or blockID for blockfiles
        // QString path = downloadsDirPath + QStirng(blockID.toHex()) + BLOCKFILE_APPENDAGE;
        QString path = downloadsDirPath + blockFileName(blockIDList->indexOf(blockID));
        QFile* blockFile = writeByteArrayToFile(path, blockData);
        blockTable->insert(blockID, blockFile);

        if(blockTable->size() == blockIDList->size())
        {
            qDebug() << "HAVE ALL BLOCKS FOR FILE " 
                     << QString(metaFileID->toHex());
            assemble();
            cleanupDownloads();
            complete = true;
        }

        return true;
    }

    return false;
}

void File::share()
{
    int readSize, i;
    char buffer[BLOCK_SIZE];    
    QString blockFilePath;
    QFile* blockFile;
    QByteArray blockData, 
               blockID, 
               metaData;

    if(!qfile->open(QIODevice::ReadOnly))
    {
        qDebug() << "COULDN'T READ FILE: " << qfile->fileName();
        return;
    }
    QDataStream in(qfile);

    // TODO: Make this code not shitty...
    i = 1;
    while(!in.atEnd())
    {
        readSize = in.readRawData(buffer, BLOCK_SIZE);
        blockData.setRawData(buffer, readSize);
        blockFilePath = tempDirPath + blockFileName(i);
        qDebug() << "BLOCKFILE " << i << "NAME: " << blockFilePath;
        blockFile = new QFile(blockFilePath);

        // write block file...move over to writeByteArrayToFile method..
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

        QCA::Hash sha("sha1");
        sha.update(blockData);
        blockID = sha.final().toByteArray();
        qDebug() << "blockID::" << blockID.toHex();
        qDebug() << "blockData::" << blockData.toHex();
        blockIDList->append(blockID);
        (*blockTable)[blockID] = blockFile;

        // update metaFile
        metaData.append(blockID);

        blockData.clear();
        blockID.clear();
        i++;

        qDebug() << "METADATA PER ITER:" << QString(metaData.toHex()) << "\n\n";
    }

    // write metaFile...move over to writeByteArrayToFile method...
    metaFile->setFileName(tempDirPath + metaFileName()); // |
    // metaFile = writeByteArray(metaFilemetaFile);      <--+
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
    QCA::Hash metaSha("sha1");
    metaSha.update(metaData);
    metaFile->close();
    qDebug() << "TESTEST::" << metaData.toHex();
    metaFileID = new QByteArray(metaSha.final().toByteArray());

    shared = true;

    qDebug() << "SHARING FILE " << fileNameOnly << " (" << fileSize << " B)";
    qDebug() << "LOCATED AT " << filePath;
    qDebug() << "TEMP DIR:" << tempDirPath;
    qDebug() << "METAFILE ID (HEX): " 
             << QCA::arrayToHex(*metaFileID) << "\n";
}

QFile* File::writeByteArrayToFile(QString filepath, QByteArray bytes)
{
    QFile* writeFile = new QFile(filepath);
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
    char buffer[BLOCK_SIZE];
    int readSize;

    if(f == NULL)
    {
        qDebug() << "NULL FILE PTR PASSED TO " 
                 << "readBytesFromFile(QFile*)!";
    }
    else if(f->open(QIODevice::ReadOnly))
    {
        QDataStream in(f);
        while(!in.atEnd())
        {
            readSize = in.readRawData(buffer, BLOCK_SIZE);
            bytes.append(QByteArray(buffer));
            qDebug() << "PER ITER " << QString::number(readSize) 
                     << ": " << QString(bytes.toHex());
        }

        qDebug() << "JUST READ FROM FILE " << f->fileName() 
                 << ": " << QString(bytes.toHex());
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
    qDebug() << "ASSEMBLING FILE WITH ID " << QString(metaFileID->toHex());

    QByteArray filebytes;

    foreach(QByteArray blockID, *blockIDList)
    {
        filebytes.append(readBytesFromFile(blockTable->value(blockID)));
    }

    qfile = writeByteArrayToFile(filePath, filebytes);
    qDebug() << "FINISHED DOWNLOADING " << qfile->fileName() 
             << " WHICH CAN BE FOUND AT " << filePath;
}

void File::cleanupDownloads()
{
    qDebug() << "CLEANING UP DOWNLOADS FOLDER FOR FILE WITH ID " 
           << QString(metaFileID->toHex());

    foreach(QByteArray blockID, blockTable->keys())
    {
        if(!(blockTable->value(blockID))->remove())
        {
            qDebug() << "ERROR REMOVING FILE " 
                     << blockTable->value(blockID)->fileName();
        }
    }
}

QString File::metaFileName()
{
    return fileNameOnly + METAFILE_APPENDAGE;
}

QString File::blockFileName(quint32 i)
{
    return fileNameOnly + "." + QString::number(i) + BLOCKFILE_APPENDAGE;
}

bool File::operator==(File other)
{
    return (this->fileID() == other.fileID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}

