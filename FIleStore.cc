#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
    , pendingDownloads(new DownloadQueue(this))
    , downloads(new QDir)
    , cycleTimer(new QTimer(this))
    , reapTimer(new QTimer(this))
{
    connect(cycleTimer, SIGNAL(timeout()), 
        this, SLOT(gotCycleChime()));
    cycleTimer->start(CYCLE_RATE);

    connect(reapTimer, SIGNAL(timeout()),
        this, SLOT(gotReapChime()));
    reapTimer->start(REAP_RATE);
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

void FileStore::setDownloadInfo(QMap<QString,DownloadStatus::Status>* di)
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
            file.share();
            sharedFiles->append(file);
            sharedFileInfo->insert(file.abspath(), file.size());
        }
    }

    Q_EMIT(refreshSharedFiles());
}

void FileStore::gotRequestFileFromPeer(QString origin, QString fileIDString)
{
    qDebug() << "REQUESTING FILE " << fileIDString 
             << " FROM " << origin; 

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

    File* newFile = new File(filePath, downloads->absolutePath(), fileID);
    Download download(newFile, origin);
    pendingDownloads->enqueue(download);
    Q_EMIT(updateDownloadInfo(filePath, DownloadStatus::CONFIRMING));

    Q_EMIT(sendDirect(request, origin));

    qDebug() << "SENT INITIAL METAFILE ID REQUEST: " << request.toString();
}

void FileStore::gotProcessBlockRequest(Message msg)
{
    qDebug() << "PROCESSING BLOCK REQUEST FROM " << msg.getOriginID() 
             << ": " << msg.toString();

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
            qDebug() << "SENDING METAFILE REPLY: " << reply.toString()
                     << " TO " << reply.getDest();
            return;
        }
        // else scan individual file blocks
        else if(file.containsBlock(data))
        {
            reply.setBlockReply(data);
            reply.setData(file.block(data));

            Q_EMIT(sendDirect(reply, reply.getDest()));
            qDebug() << "SENDING BLOCK REPLY: " << reply.toString() 
                     << " TO " << reply.getDest();
            return;
        }
    }
}

void FileStore::gotProcessBlockReply(Message reply)
{    
    qDebug() << "PROCESSING BLOCK REPLY FROM " << reply.getOriginID() 
             << ": " << reply.toString();

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
    else if(query->fileObject()->fileID() == blockID && query->needsMetaData())
    {   // got dl confirmation. create dl obj and init dl
        query->addMetaData(blockData);
        pendingDownloads->enqueue(*query);
        enDequeuePendingDownloadQueue();
    }
    else if(query->needsBlock(blockID))
    {
        query->addBlockData(blockID, blockData);
        pendingDownloads->enqueue(*query);
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

void FileStore::gotCycleChime()
{
    cyclePendingDownloadQueue();
}

void FileStore::gotReapChime()
{
    pendingDownloads->reap();
}

void FileStore::gotUpdateDownloadInfo(QString fn, DownloadStatus::Status status)
{
    downloadInfo->insert(fn, status);
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

    downloads = new QDir(tempdir->absolutePath() + "/" + DOWNLOADS_DIR_NAME);
    if(!downloads->exists())
    {
        if(downloads->mkpath(downloads->absolutePath() + "/"))
        {
            qDebug() << "SUCCESSFULLY CREATED DOWNLOADS DIR: " 
                     << downloads->absolutePath();
        }
        else
        {
            qDebug() << "UNABLE TO CREATE DOWNLOADS DIR " << downloads->absolutePath()
                     << "! CHECK PERMISSIONS!";
        }
    }
}

bool FileStore::enDequeuePendingDownloadQueue()
{
    qDebug() << "ENDEQUEUE PENDING DOWNLOADQUEUE";

    if(!pendingDownloads->isEmpty())
    {
        Download head = pendingDownloads->dequeue();

        qDebug() << "POP PENDING DOWNLOADQUEUE HEAD: "
                 << head.toString();

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
            qDebug() << "\tSENT BLOCK_REQUEST " << QString(*i->toHex())
                     << " TO " << request.getOriginID();
        }

        if(head.isAlive())
        {
            qDebug() << "RE-QUEUE'ING HEAD " << head.toString();

            pendingDownloads->enqueue(head);
            return true;
        }

        return false;
    }
}

void FileStore::cyclePendingDownloadQueue()
{
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
{
    begin();
}

FileStore::Download::~Download()
{}

QString FileStore::Download::toString()
{
    QString qstr = "-------- Download --------\n";
    qstr += "peer originID: " + peerID + "\n";
    qstr += "DownloadStatus: " + QString::number(downloadStatus) + "\n";
    qstr += "File:\n" + file->toString() + "\n\n";
    qstr += "--------------------------\n\n";

    return qstr;
}

File* FileStore::Download::fileObject()
{
    return file;
}

QString FileStore::Download::peer()
{
    return peerID;
}

DownloadStatus::Status FileStore::Download::status()
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

bool FileStore::Download::needsMetaData()
{
    return downloadStatus == (DownloadStatus::INIT | 
                              DownloadStatus::CONFIRMING);
}

bool FileStore::Download::needsBlock(QByteArray blockID)
{
    return blocksNeeded().contains(blockID);
}

bool FileStore::Download::isAlive()
{
    return downloadStatus == (DownloadStatus::INIT |
                              DownloadStatus::CONFIRMING |
                              DownloadStatus::DOWNLOADING);
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

    confirm();
}

void FileStore::Download::addBlockData(QByteArray blockID, QByteArray blockData)
{
    if(!file->hasBlock(blockID) && file->containsBlock(blockID) && isAlive())
    {
        file->addBlock(blockID, blockData);
        if(file->isComplete())
        {
            complete();
        }
    }
}

void FileStore::Download::begin()
{
    downloadStatus = DownloadStatus::CONFIRMING;
}

void FileStore::Download::confirm()
{
    downloadStatus = DownloadStatus::DOWNLOADING;
}

void FileStore::Download::kill()
{
    downloadStatus = DownloadStatus::FAILED;
}

void FileStore::Download::complete()
{
    downloadStatus = DownloadStatus::COMPLETED;
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

FileStore::Download FileStore::DownloadQueue::dequeue()
{
    return (isEmpty() ? Download() : ((Download) takeFirst()));
}

void FileStore::DownloadQueue::enqueue(Download d)
{
    append(d);
}

FileStore::Download* FileStore::DownloadQueue::search(QByteArray blockID)
{
    QList<Download>::iterator i;
    for(i = this->begin(); i != this->end(); ++i)
    {
        if(i->fileObject()->containsBlock(blockID) ||
           i->fileObject()->fileID() == blockID)
        {
            return &(*i);
        }
    }

    return NULL;    // blockID not found in any pending DL's blocks!
}

void FileStore::DownloadQueue::upadateDownloadInfo(QString path, 
                                    DownloadStatus::Status status)
{
    Q_EMIT(filestore->updateDownloadInfo(path, status));
}

void FileStore::DownloadQueue::reap()
{
    if(!isEmpty())
    {
        QList<Download>::iterator i;
        for(i = this->begin(); i != this->end(); ++i)
        {
            switch(i->status())
            {
                case DownloadStatus::INIT:
                    filestore->updateDownloadInfo(i->fileObject()->abspath(), 
                                          DownloadStatus::INIT);
                    qDebug() << "DOWNLOAD FLOW ERROR! SHOULD NOT BE IN "
                             << "INIT STATE AND ON DOWNLOADQUEUE!";
                    break;
                case DownloadStatus::CONFIRMING:
                    filestore->updateDownloadInfo(i->fileObject()->abspath(), 
                                          DownloadStatus::CONFIRMING);
                    break;
                case DownloadStatus::DOWNLOADING:
                    filestore->updateDownloadInfo(i->fileObject()->abspath(), 
                                          DownloadStatus::DOWNLOADING);
                    break;
                case DownloadStatus::COMPLETED:
                    filestore->updateDownloadInfo(i->fileObject()->abspath(), 
                                          DownloadStatus::COMPLETED);
                    erase(i);
                    break;
                case DownloadStatus::FAILED:
                    filestore->updateDownloadInfo(i->fileObject()->abspath(), 
                                          DownloadStatus::FAILED);
                    erase(i);
                    break;
                case DownloadStatus::NONE:
                    filestore->updateDownloadInfo(i->fileObject()->abspath(), 
                                          DownloadStatus::NONE);
                    break;
                default:
                    qDebug() << "INVALID DOWNLOADSTATUS " << QString(i->status());
            }
        }
    }
}

bool FileStore::DownloadQueue::isEmpty()
{
    return empty();
}