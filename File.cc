#include "File.hh"

File::File(QString absolutepath, QString tempdirpath)
    : blocks(new QList<QFile>)
    , blockHashes(new QList<QFile>)
{
    if(absolutepath.isEmpty())
    {
        qDebug() << "FILE CONSTRUCTED FROM EMPTY PATH!";
        return;
    }

    qfile = new QFile(absolutepath);
    if (!qfile.open(QIODevice::ReadOnly)) {
        qDebug() << "FILE " << absolutepath << "DOESN'T EXIST!";
        return;
    }

    fileName = absolutepath.split("/").last();
    absFilePath = qfile.fileName();
    fileSize = qfile.size();

    fileID = QString::number((qrand() % ID_MAX) + 1);

    tempDirPath = tempdirpath + 
                 (tempdirpath.endsWith("/") ? "" : "/");

    generateBlocks();
    hashBlocks();
    generateMetaFile();
    hashMetafile();
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

void File::generateBlocks(File file)
{
    QDataStream fileIn(qfile), 
                blockOut;
    QFile block;
    QString blockName;
    char buffer[BLOCK_SIZE];
    int readSize, i;

    i = 1;
    while(!fileIn.atEnd())
    {
        readSize = fileIn.readRawData(buffer, blockSize);
        blockName = tempDirPath + fileName + "." + i + ".block";
        block = QFile(blockName);
        if (block.open(QIODevice::WriteOnly))
        {
            blockOut = QDataStream(&block);
            blockOut.writeRawData(buffer, readSize);
            blockOut.close();
        }
        else
        {
            qCritical() << "COULDN'T CREATE BLOCK "<< i << "!";
        }
        blocks.append(block);

        i++;
    }

    // anything else? make sure this is complete.
}

void File::hashBlocks(File file)
{
    QCA::Hash shaHash("sha256");
}

void File::generateMetaFile(File file)
{}

void File::hashMetafile(File file)
{}

bool File::operator==(File other)
{
    return (this->ID() == other.ID());
} 

bool File::operator!=(File other)
{
    return !(*this == other);
}



