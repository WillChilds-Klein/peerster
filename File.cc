#include "File.hh"

File::File(QString absolutepath, QString tempdirpath)
    : blocks(new QList<QFile>)
    , blockHashes(new QList<QByteArray>)
{
    if(absolutepath.isEmpty())
    {
        qDebug() << "FILE CONSTRUCTED FROM EMPTY PATH!";
        return;
    }

    qfile.setFileName(absolutepath);
    if (!qfile.open(QIODevice::ReadOnly)) {
        qDebug() << "FILE " << absolutepath << "DOESN'T EXIST!";
        return;
    }

    fileName = absolutepath.split("/").last();
    absFilePath = qfile.fileName();
    fileSize = qfile.size();

    fileID = QString::number((qrand() % FILE_ID_MAX) + 1);

    tempDirPath = tempdirpath + 
                 (tempdirpath.endsWith("/") ? "" : "/");

    processFile();
    qfile.close();
}

File::~File()
{}

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
    return tempDirPath + fileName + "." + i + ".block";
}

void File::processFile()
{
    QCA::Hash sha("sha256");
    QDataStream fileIn(&qfile), 
                *blockOut;
    QFile* block;
    QString blockName;
    QByteArray blockData, blockHash, metaData;
    char buffer[BLOCK_SIZE];
    int readSize, i;

    i = 1;
    while(!fileIn.atEnd())
    {
        blockName = blockFileName(i);
        block = new QFile(blockName);
        readSize = fileIn.readRawData(buffer, BLOCK_SIZE);

        // write block.
        if (block->open(QIODevice::WriteOnly))
        {
            blockOut = new QDataStream(&block);
            blockOut.writeRawData(buffer, readSize);
            delete(blockOut);
            block->close();
        }
        else
        {
            qCritical() << "COULDN'T CREATE BLOCK "<< i << " FOR " 
                                                   << fileName << "!";
        }
        blocks->append(*block);

        // hash block
        blockData = block->readAll();
        blockHash = sha.hash(blockData).toByteArray();
        blockHashes->append(blockHash);

        // update metafile
        metaData.append(blockHash);

        delete(block);
        i++;
    }

    // write metafile
    metafile.setFileName(tempDirPath + fileName + ".meta");
    if(metafile.open(QIODevice::WriteOnly))
    {
        metafile.writeData(metaData);
        metafile.close();
    }
    else
    {
        qCritical() << "COULDN'T CREATE METAFILE FOR "<< fileName << "!";
    }

    // hash metafile
    metafileHash = sha.hash(metaData).toByteArray();
}

bool File::operator==(File other)
{
    return (this->ID() == other.ID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}



