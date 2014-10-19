#include "File.hh"

File::File(QString absolutepath, QString tempdirpath)
    : qfile(new QFile)
    , metaFile(new QFile)
    , blocks(new QList<QFile*>)
    , blockHashes(new QList<QByteArray>)
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
    fileSize = qfile->size();

    fileID = QString::number((qrand() % FILE_ID_MAX) + 1);

    tempDirPath = tempdirpath + 
                 (tempdirpath.endsWith("/") ? "" : "/");

    qDebug() << "SHARING FILE " << fileName << " (" 
             << fileSize << " B)";
    qDebug() << "LOCATED AT " << absFilePath;
    qDebug() << "FILE ID: " << fileID;
    qDebug() << "TEMP DIR:" << tempDirPath;

    processFile();
}

File::~File()
{
    // qfile->close();
    // delete(qfile);
    // qDeleteAll(blocks);
    // delete(blocks);
    // delete(blockHashes);
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

QString File::ID()
{
    return fileID;
}

QString File::blockFileName(quint32 i)
{
    return tempDirPath + fileName + "." + QString::number(i) + ".block";
}

void File::processFile()
{
    QCA::Hash sha("sha1");
    QDataStream fileInStream(qfile);
    QString blockName;
    QByteArray blockData, blockHash, metaData;
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
            qCritical() << "COULDN'T CREATE BLOCK "<< i << " FOR " 
                                                   << fileName << "!";
            return;
        }
        blocks->append(blockFile);

        sha.update(blockFile);
        blockHash = sha.final().toByteArray();
        blockHashes->append(blockHash);

        // update metaFile
        metaData.append(blockHash);

        blockData.clear();
        blockHash.clear();
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
    metaFileHash = sha.final().toByteArray();
    metaFile->close();
}

bool File::operator==(File other)
{
    return (this->ID() == other.ID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}



