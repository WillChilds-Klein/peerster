#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
    , confirmingDownloads(new DownloadQueue(this))
    , pendingDownloads(new DownloadQueue(this))
    , completedDownloads(new DownloadQueue(this))
    , failedDownloads(new DownloadQueue(this))
    , downloads(new QDir)
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
    // TODO: deprecate once moved over to instanceID-constructed model.
    ID = id;
    makeTempdir();
}

void FileStore::setSharedFileInfo(QMap<QString,quint32>* sfi)
{
    sharedFileInfo = sfi;
}

void FileStore::setDownloadInfo(QMap<QString,DownloadStatus>* di)
{
    downloadInfo = di;
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

void FileStore::gotRequestFileFromPeer(QString origin, QString fileIDString)
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

    File* newFile = new File(filePath, tempdir->absolutePath(), fileID);
    Download download(newFile, origin);
    confirmingDownloads->enqueue(download);
    Q_EMIT(updateDownloadInfo(filePath, DownloadStatus::CONFIRMING));

    Q_EMIT(sendDirect(request, origin));

    qDebug() << "SENT INITIAL METAFILE ID REQUEST: " << request.toString();
}

void FileStore::gotProcessBlockRequest(Message msg)
{
    QByteArray data = msg.getBlockRequest();
    QString blockPath;

    Message reply;
    reply.setType(TYPE_BLOCK_REPLY);
    reply.setOriginID(ID);
    reply.setDest(msg.getOriginID());
    reply.setHopLimit(BLOCK_HOP_LIMIT);

    foreach(File file, *sharedFiles)
    {
        // check to see if request is initiating download
        if(data == file.fileID())
        {
            reply.setBlockReply(file.fileID());
            reply.setData(file.metadata());

            Q_EMIT(sendDirect(reply, reply.getDest()));
            return;
        }
        // else scan individual file blocks
        else if(file.containsBlock(data))
        {
            reply.setBlockReply(data);
            reply.setData(file.block(data));

            Q_EMIT(sendDirect(reply, reply.getDest()));
            return;
        }
    }
}

void FileStore::gotProcessBlockReply(Message reply)
{    
    QFile* file;
    QByteArray blockID = reply.getBlockReply(),
               blockData = reply.getData();

    Download* query = pendingDownloads->search(blockID);

    if(query == NULL)
    {
        qDebug() << "GOT REPLY FOR NON-PENDING OR UNREQUESTED DOWNLOAD";
        // TODO: differentiate between:
        //  1. unrequested block
        //  2. block for failed download
        //  3. block for complete file
    }
    else if(query->fileID() == blockID && query->needsMetaData())
    {   // got dl confirmation. create dl obj and init dl
        query->addMetaData(blockData);
        enDequeuePendingDownloadQueue();
    }
    else if(query->needs(blockID))
    {
        query->addBlockData(blockID, blockData);
        enDequeuePendingDownloadQueue();
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
    cyclePendingDownloadQueue();
}

void FileStore::gotUpdateDownloadInfo(QString fn, DownloadStatus status)
{
    downloadInfo->[fn] = status;
    // (*downloadInfo)[fn] = status;
    // downloadInfo->insert(fn, status);
    Q_EMIT(refreshDownloadInfo());
}

void FileStore::makeTempdir()
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

bool FileStore::enDequeuePendingDownloadQueue()
{
    qDebug() << "ENDEQUEUE PENDING DOWNLOADQUEUE";

    if(!pendingDownloads->isEmpty())
    {
        pendingDownloads->reap();

        qDebug() << "POP PENDING DOWNLOADQUEUE HEAD";

        Download head = pendingDownloads->dequeue();

        Message request;
        request.setType(TYPE_BLOCK_REQUEST);
        request.setDest(head.peer());
        request.setOriginID(ID);
        request.setHopLimit((quint32) BLOCK_HOP_LIMIT);
        
        QList<QByteArray> needed = head.blocksNeeded();
        QList<QByteArray>::iterator i;
        for(i = needed.begin(); i != needed.end(); ++i)
        {
            head.touch(*i);     // increment # times each request sent.
            request.setBlockRequest(*i);
            Q_EMIT(sendDirect(request, request.getDest()));
            qDebug() << "\tSENT BLOCK_REQUEST " << *i
                     << " TO " << request.getOriginID();
        }

        if(head.isAlive())
        {
            qDebug() << "RE-QUEUE'ING HEAD";

            pendingDownloads->enqueue(head);
            return true;
        }

        return false;
    }
}

void FileStore::cyclePendingDownloadQueue()
{
    pendingDownloads->reap();

    int size = pendingDownloads->size();
    bool headReQueued;

    for(int i = 0; i < size; i++)
    {
        headReQueued = enDequeuePendingDownloadQueue();
        if(!headReQueued)
        {
            size--;
        }
    }

    qDebug() << "CYCLE PENDING DOWNLOADQUEUE";
}

// =========================FileStore::Download============================= //

FileStore::Download::Download()
    : file(NULL)
    , peerID("")
    , downloadStatus(DownloadStatus::NONE)
{}

FileStore::Download::Download(File* f, QString p)
    : file(f)
    , peerID(p)
    , downloadStatus(DownloadStatus::INIT)
{}

FileStore::Download::~Download()
{}

File* FileStore::Download::fileObject()
{
    return file;
}

QString FileStore::Download::peer()
{
    return peerID;
}

DownloadStatus FileStore::Download::status()
{
    return downloadStatus;
}

QList<QByteArray> FileStore::Download::blocksNeeded()
{
    QList<QByteArray> needed,
                      blockIDs = file->blockIDs();

    QList<QByteArray>::iterator i;
    for(i = blockIDs.begin(); i != blockIDs.end(); ++i)
    {
        if(!file->hasBlock(*i))
        {
            needed.append(*i);
        }
    }

    return needed;
}

bool FileStore::Download::needs(QByteArray blockID)
{
    return blocksNeeded().contains(blockID);
}

bool FileStore::Download::isAlive()
{
    return downloadStatus == (DownloadStatus::PENDING | 
                              DownloadStatus::CONFIRMING);
}

void FileStore::Download::touch(QByteArray blockID)
{
    insert(blockID, value(blockID)+1);

    if(value(blockID) > BLOCK_REQUEST_LIMIT)
    {
        kill();
        return;
    }
}

void FileStore::Download::addMetaData(QByteArray metaDataBytes)
{
    file->addMetaData(metaDataBytes);

    QList<QByteArray> blockIDs = file->blockIDs();
    QList<QByteArray>::iterator i;
    for(i = blockIDs.begin(); i != blockIDs.end(); ++i)
    {
        insert(*i, 0);
    }

    begin();
}

void FileStore::Download::addBlockData(QByteArray blockID, QByteArray blockData)
{
    if(!file->hasBlock(blockID) && file->contains(blockID) && isAlive())
    {
        file->addBlock(blockID, blockData);
        if(file->isComplete())
        {
            complete();
        }
    }
}

void FileStore::Download::kill()
{
    downloadStatus = DownloadStatus::FAILED;
}

void FileStore::Download::complete()
{
    downloadStatus = DownloadStatus::COMPLETE;
}

// =========================FileStore::DownloadQueue======================== //

FileStore::DownloadQueue::DownloadQueue(FileStore* fs)
    : filestore(fs)
{}

FileStore::DownloadQueue::~DownloadQueue()
{}

quint32 FileStore::DownloadQueue::size()
{
    return size();
}

Download FileStore::DownloadQueue::dequeue()
{
    return isEmpty() ? Download() : takeFirst();
}

void FileStore::DownloadQueue::enqueue(Download d)
{
    append(d);
}

void FileStore::DownloadQueue::search(QByteArray blockID)
{
    QList<Download>::iterator i;
    for(i = this->begin(); i != this->end(); ++i)
    {
        if(i->containsBlock(blockID))
        {
            return &(*i);
        }
    }

    return NULL;    // blockID not found in any pending DL's blocks!
}

void FileStore::DownloadQueue::reap()
{
    if(!isEmpty())
    {
        QList<Download>::iterator i;
        for(i = this->begin(); i != this->end(); ++i)
        {
            if(!i->isAlive() && i->isEmpty())
            {
                Q_EMIT(updateDownloadInfo(i->fileObject()->filePath(), 
                                          DownloadStatus::COMPLETE));
                erase(i);
            }
            else if(!i->isAlive() && !i->isEmpty())
            {
                Q_EMIT(updateDownloadInfo(i->fileObject()->filePath(), 
                                          DownloadStatus::FAILED));
                erase(i);
            }
        }
    }
}

bool FileStore::DownloadQueue::isEmpty()
{
    return empty();
}




