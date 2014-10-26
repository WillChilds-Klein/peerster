#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
    , pendingFileDownloads(new DownloadQueue)
    , downloads(new QQueue<Download>)
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

void FileStore::setDownloadInfo(QMap<QString,DownloadStatus> di)
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

    File newFile = new File(filePath, tempPath);
    Download download(newFile, origin);
    pendingDownloads->enqueue(download);
    Q_EMIT(updateDownloadInfo(filepath, DownloadStatus::PENDING));

    Q_EMIT(sendDirect(request, origin));

    qDebug() << "SENT INITIAL METAFILE ID REQUEST: " << request.toString();
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
    pendingDownloads->processBlock(reply.getBlockReply(), reply.getData());
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
    if(!pendingDownloads->empty())
    {
        pendingDownloads->reap();

        Download head = pendingDownloads->dequeue();

        Message request;
        request.setType(TYPE_BLOCK_REQUEST);
        request.setDest(head.peer());
        request.setOriginID(ID);
        request.setHopLimit((quint32) BLOCK_HOP_LIMIT);
        
        QList<QByteArray> needed = head.IDsNeeded();
        QList<QByteArray>::iterator i;
        for(i = needed.begin(); i != needed.end(); ++i)
        {   // update # times each request sent.
            head.touch(*i);
            request.setBlockRequest(blockID);
            Q_EMIT(sendDirect(request, request.getDest()));
        }

        if(head.isAlive())
        {
            enqueue(head);
        }
    }
}

void gotUpdateDownloadInfo(QString fn, DownloadStatus status)
{
    downloadInfo[fn] = status;
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

FileStore::Download::Download(File* f, QString p)
    : file(f)
    , peer(p)
    , alive(true)
{}

FileStore::Download::~Download()
{}

File* FileStore::Download::file()
{
    return file;
}

QString FileStore::Download::peer()
{
    return peer;
}

bool FileStore::Download::isAlive()
{
    return alive;
}

void FileStore::Download::touch(QByteArray blockID)
{
    QList<QByteArray>::iterator i;
    for(i = keys().begin(); i != keys().end(); ++i)
    {
        (*this)[blockID]++;

        if((*this)[blockID] > BLOCK_REQUEST_LIMIT)
        {
            kill();
            return;
        }
    }
}

void FileStore::Download::addBlockID(QByteArray blockID)
{
    if(!contains(blockID) && isAlive())
    {
        (*this)[blockID] = 0;
    }
}

void FileStore::Download::removeBlockID(QByteArray blockID)
{
    if(!empty())
    {
        removeOne(blockID);

        if(empty())
        { 
            kill(); // file successfully downloaded!!
        }
    }
    else
    {
        kill();    // file successfully downloaded!!
    }
}

void FileStore::Download::addBlockToFile(QByteArray blockID, QByteArray blockData)
{
    if(!hasBlock(blockID) && contains(blockID) && isAlive())
    {
        file->addBlockToFile(blockID, blockData);
    }
}

void FileStore::Download::kill()
{
    alive = false;
}

FileStore::DownloadQueue::DownloadQueue(FileStore* fs)
    : filestore(fs)
{}

FileStore::DownloadQueue::~DownloadQueue()
{}

Download FileStore::DownloadQueue::dequeue()
{
    return takeFirst();
}

void FileStore::DownloadQueue::enqueue(Download d)
{
    append(d);
}

void FileStore::DownloadQueue::processBlock(QByteArray blockID, QByteArray blockData)
{    
    QFile* file;
    quint32 blockIndex;

    QList<Download>::iterator i;
    for(i = this->begin(); i != this->end(); ++i)
    {
        if(i->isAlive())
        {
            file = i->file();

            // check against each fileID
            if(bockID == file->ID())
            {
                // break down metafile
                for(int j = 0; j < blockData.size(); j += HASH_SIZE)
                {
                    blockIndex = (j / HASH_SIZE);
                    if(blockData.mid(j,j+HASH_SIZE) > 0)
                    {
                        i->addBlockID(blockIndex, blockData.mid(j,j+HASH_SIZE));
                    }
                }

                i->removeBlockID(blockID);
                return;
            }
            // else scan individual file blocks
            else if(file.containsBlock(bockID) && !file.hasBlock(blockID))
            {
                file->addBlockToFile(bockID, blockData);

                i->removeBlockID(blockID);
                return;
            }
        }
    }
}

void FileStore::DownloadQueue::reap()
{
    QList<Download>::iterator i;
    for(i = this->begin(); i != this->end(); ++i)
    {
        if(!i->isAlive() && i->empty())
        {
            Q_EMIT(updateDownloadInfo(i->file()->filePath(), DownloadStatus::COMPLETE));
            erase(i);
        }
        else if(!i->isAlive())
        {
            Q_EMIT(updateDownloadInfo(i->file()->filePath(), DownloadStatus::FAILED));
            erase(i);
        }
    }
}




