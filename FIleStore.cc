#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
    , pendingRequests(new QHash<QString,quint32>)
    , pendingFileDownloads(new QList< QPair<File,QString> >)
    , downloads(new QDir(QDir::currentPath() + DOWNLOADS_DIR_NAME))
    , blockRequestTimer(new QTimer(this))
{
    // downloads = new QDir(QDir::currentPath() + DOWNLOADS_DIR_NAME);
    connect(blockRequestTimer, SIGNAL(chime()), 
        this, SLOT(gotBlockRequestChime()));
    blockRequestTimer->start(BLOCK_REQUEST_RATE);
}

FileStore::~FileStore()
{}

void FileStore::setID(QString id)
{
    ID = id;

    makeTempdir();
}

void FileStore::setSharedFileInfo(QMap<QString,quint32>* sfi)
{
    sharedFileInfo = sfi;
}

void FileStore::gotProcessFilesToShare(QStringList absfilepaths)
{
    foreach(QString filepath, absfilepaths)
    {
        if(!(sharedFileInfo->keys().contains(filepath)))
        {
            File file = File(filepath, tempdir->absolutePath());
            sharedFiles->append(file);
            sharedFileInfo->insert(file.abspath(), file.size());
        }
    }

    Q_EMIT(refreshSharedFiles());
}

void FileStore::gotRequestFile(QString origin, QString fileIDString)
{
    QByteArray fileID = QByteArray::fromHex(fileIDString.toLatin1());
    QString fileName = "download-" +  QString::number((qrand() % 1000) + 1);
    QString filePath = downloads->absolutePath() + 
                      (downloads->absolutePath().endsWith("/") ? "" : "/") + 
                       fileName;
    
    Message request;
    request.setType(TYPE_BLOCK_REQUEST);
    request.setDest(origin);
    request.setOriginID(ID);
    request.setHopLimit((quint32) BLOCK_HOP_LIMIT);
    request.setBlockRequest(fileID);

    File file = File(filePath, tempdir->absolutePath(), fileID);

    pendingFileDownloads->append(QPair(file, origin));

    Q_EMIT(sendDirect(request, origin));
    (*pendingRequests)[fileID] = 1;

    qDebug() << "SENT INITIAL REQUEST: " << request.toString();
}

void FileStore::gotProcessBlockRequest(Message msg)
{
    QByteArray data = msg.getBlockRequest();
    Message reply;
    reply.setType(TYPE_BLOCK_REPLY);
    QString blockPath;

    foreach(File file, *sharedFiles)
    {
        // check to see if request is initiating download
        if(data == file.ID())
        {
            reply.setOriginID(ID);
            reply.setDest(msg.getOriginID());
            reply.setHopLimit(BLOCK_HOP_LIMIT);
            reply.setBlockReply(file.ID());
            reply.setData(file.metadata());

            Q_EMIT(sendDirect(reply, reply.getDest()));
            return;
        }
        // else scan individual file blocks
        else if(file.containsBlock(data))
        {
            reply.setOriginID(ID);
            reply.setDest(msg.getOriginID());
            reply.setHopLimit(BLOCK_HOP_LIMIT);
            reply.setBlockReply(data);
            reply.setData(file.block(data));

            Q_EMIT(sendDirect(reply, reply.getDest()));
            return;
        }
    }
}

void FileStore::gotProcessBlockReply(Message reply)
{    
    QByteArray metaData,
               blockID = reply.getBlockReply();
    QList<QByteArray> blockIDs;
    QString blockPath;
    quint32 blockIndex;

    foreach(File file, *pendingFileDownloads)
    {
        // check to see if request is confirming download
        // init by replying with requested file's fileID
        if(bockID == file.ID())
        {
            // break down metafile
            metaData = reply.metadata();
            for(int i = 0; i < metaData.size(); i += HASH_SIZE)
            {
                blockIndex = (i / HASH_SIZE);
                if(metaData.mid(i,i+HASH_SIZE) > 0)
                {
                    file.addBlockID(blockIndex, metaData.mid(i,i+HASH_SIZE));
                }
            }

            pendingRequests->remove(blockID);
            return;
        }
        // else scan individual file blocks
        else if(file.containsBlock(bockID))
        {
            if(!file.hasBlock(blockID))
            {
                file.addBlock(bockID, reply.getData());
                pendingRequests->remove(blockID);
            }

            if(file.isComplete())
            {
                pendingFileDownloads->removeOne(file);
            }

            return;
        }
    }

}

void FileStore::gotProcessSearchRequest(Message msg)
{
    // TODO
}

void FileStore::gotProcessSearchReply(Message msg)
{
    // TODO
}

void FileStore::gotBlockRequestChime()
{
    if(!pendingFileDownloads->empty())
    {
        File head = pendingFileDownloads->first().first;
        QString peer = pendingFileDownloads->first().second;

        

        Message request;
        request.setType(TYPE_BLOCK_REQUEST);
        request.setDest(peer);
        request.setOriginID(ID);
        request.setHopLimit((quint32) BLOCK_HOP_LIMIT);
        request.setBlockRequest();
    }
}

void FileStore::makeTempdir() path()
{
    tempdir = new QDir(QDir::tempPath() + "/peerster-" + ID);
    
    if(!tempdir->exists())
    {
        if(tempdir->mkpath(tempdir->absolutePath()))
        {
            qDebug() << "SUCESSFULLY CREATED TEMP DIR " << tempdir->absolutePath();
        }
        else
        {
            qDebug() << "UNABLE TO CREATE TEMP DIR " << tempdir->absolutePath()
                     << "! CHECK PERMISSIONS!";
        }
    }

}

bool maxRequestsMade(QByteArray blockID)
{
    return pendingRequests[blockID] >= BLOCK_REQUEST_LIMIT;
}

