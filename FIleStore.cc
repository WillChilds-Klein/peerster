#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
    , pendingDownloads(new DownloadQueue(this))
    , pendingSearches(new QHash<int,Search*>)
    , popTimer(new QTimer(this))
    , reapTimer(new QTimer(this))
{
    connect(popTimer, SIGNAL(timeout()), 
        this, SLOT(gotPopChime()));
    popTimer->start(POP_RATE);

    connect(reapTimer, SIGNAL(timeout()),
        this, SLOT(gotReapChime()));
    reapTimer->start(REAP_RATE);

    downloads = new QDir(QDir::currentPath() +
                        (QDir::currentPath().endsWith("/") ? "" : "/") +
                         DOWNLOADS_DIR_NAME + "/");

    if(downloads->exists())
    {
        qDebug() << "DOWNLOADS DIR ALREADY EXISTS! EMPTYING IT.";

        QDir dir(downloads->absolutePath());
        dir.setNameFilters(QStringList() << "*");
        dir.setFilter(QDir::Files);
        
        foreach(QString dirFile, dir.entryList())
        {
            dir.remove(dirFile);
        }
    }
    else if(downloads->mkpath(downloads->absolutePath() + "/"))
    {
        qDebug() << "SUCCESSFULLY CREATED DOWNLOADS DIR: " 
                 << downloads->absolutePath();
    }
    else
    {
        qDebug() << "UNABLE TO CREATE DOWNLOADS DIR " 
                 << downloads->absolutePath() << "! CHECK PERMISSIONS!";
    }
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

void FileStore::setSearchResults(QMultiHash< QString,QPair<QString,QByteArray> >* sr)
{
    searchResults = sr;
}

void FileStore::gotProcessFilesToShare(QStringList absfilepaths)
{
    QStringList::iterator i;
    for(i = absfilepaths.begin(); i != absfilepaths.end(); ++i)
    {
        QString filepath = *i;
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

void FileStore::gotSearchForKeywords(QString keywords)
{
    if(searchIDByKeywords(keywords) >= 0)
    {
        qDebug() << "SEARCH IS ALREADY PENDING!";
        return;
    }

    Message* msg = new Message();

    msg->setType(TYPE_SEARCH_REQUEST);
    msg->setOriginID(ID);
    msg->setSearch(keywords);
    msg->setBudget(SEARCH_BUDGET_INIT);

    int searchID = startTimer(BUDGET_INC_RATE);

    Search* search = new Search(msg, searchID);

    pendingSearches->insert(searchID, search);
    Q_EMIT(postToInbox(*msg, Peer()));
}

void FileStore::gotRequestFileFromPeer(QString origin, QPair<QString,QByteArray> match)
{
    qDebug() << "REQUESTING FILE " << match.first << " (FileID: " 
             << QString((match.second).toHex())
             << ") FROM " << origin; 

    QString fileName = match.first;
    QByteArray fileID = match.second;

    QString filePath = downloads->absolutePath() + 
                      (downloads->absolutePath().endsWith("/") ? "" : "/") + 
                       fileName;

    File* newFile = new File(filePath, downloads->absolutePath(), fileID);
    Download* newDownload = new Download(newFile, origin);
    
    pendingDownloads->enqueue(newDownload);
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

    QByteArray blockID = reply.getBlockReply(),
               blockData = reply.getData();

    Download* query = pendingDownloads->search(blockID);

    if(query == NULL)
    {
        qDebug() << "GOT REPLY FOR NON-PENDING OR UNREQUESTED DOWNLOAD";
    }
    else if(query->fileObject()->fileID() == blockID && query->needsMetaData())
    {   // got dl confirmation.
        query->addMetaData(blockData);
    }
    else if(query->needsBlock(blockID))
    {
        query->addBlockData(blockID, blockData);
    }
}

void FileStore::gotProcessSearchRequest(Message request)
{
    QVariantList matches;
    QByteArray matchIDs;

    QStringList keywords = request.getSearch().split(" ");

    foreach(QString keyword, keywords)
    {
        foreach(File file, *sharedFiles)
        {
            if(file.name().contains(keyword, Qt::CaseInsensitive))
            {
                matches.append(file.name());
                matchIDs.append(file.fileID());
                qDebug() << "FOUND FILE MATCH " << file.name()
                         << " TO QUERY KEYWORD " << keyword;
            }
        }
    }

    if(matches.size() > 0)
    {
        Message reply;

        reply.setType(TYPE_SEARCH_REPLY);
        reply.setOriginID(ID);
        reply.setDest(request.getOriginID());
        reply.setHopLimit(SEARCH_REPLY_HOP_LIMIT);
        reply.setSearchReply(request.getSearch());
        reply.setMatchNames(matches);
        reply.setMatchIDs(matchIDs);

        Q_EMIT(sendDirect(reply, reply.getDest()));
    }
}

void FileStore::gotProcessSearchReply(Message reply)
{
    QPair<QString,QByteArray> match;
    QVariantList matchNames = reply.getMatchNames();
    QList<QByteArray> matchIDs;

    for(int i = 0; i < reply.getMatchIDs().size(); i += BLOCK_SIZE)
    {
        matchIDs.append(reply.getMatchIDs().mid(i,i+BLOCK_SIZE));
    }

    int searchID = searchIDByKeywords(reply.getSearchReply());
    
    if(searchID >= 0 && searchResults->size() < SEARCH_RESULTS_LIMIT && 
       !searchResults->contains(reply.getOriginID()))
    {
        for(int i = 0; i < matchNames.size(); ++i)
        {
            match.first = matchNames.at(i).toString();
            match.second = matchIDs.at(i);
            searchResults->insert(reply.getOriginID(), match);
        }
        
        Q_EMIT(refreshSearchResults());
    }

}

void FileStore::gotPopChime()
{
    if(pendingDownloads->size() >= CYCLE_THRESHOLD)
    {
        qDebug() << "PENDING DL'S SIZE: " 
                 << QString::number(pendingDownloads->size());
        cyclePendingDownloadQueue();
    }
    else if(!pendingDownloads->isEmpty())
    {
        enDequeuePendingDownloadQueue();
    }
}

void FileStore::gotReapChime()
{
    pendingDownloads->reap();
}

void FileStore::gotUpdateDownloadInfo(Download dl)
{
    downloadInfo->insert(dl.fileObject()->abspath(), dl.status());
    Q_EMIT(refreshDownloadInfo());
}

void FileStore::timerEvent(QTimerEvent* event)
{
    int searchID = event->timerId();
    Search* search = pendingSearches->value(searchID);

    if(search == NULL)
    {
        qDebug() << "REQUESTED SEARCH NOT PENDING!";
        return;
    }
    else if(search->budget() >= SEARCH_BUDGET_LIMIT || 
       search->results() >= SEARCH_RESULTS_LIMIT)
    {
        killSearch(searchID);
    }
    else
    {
        Q_EMIT(postToInbox(search->message(), Peer()));

        search->incrementBudget();
        if(search->budget() >= SEARCH_BUDGET_LIMIT)
        {
            killSearch(searchID);
        }
    }    
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

int FileStore::searchIDByKeywords(QString keywords)
{
    QHash<int,Search*>::iterator i;
    for(i = pendingSearches->begin(); i != pendingSearches->end(); ++i)
    {
        if(i.value()->keywords() == keywords)
        {
            return i.key();
        }
    }

    return -1;
}

void FileStore::killSearch(int searchID)
{
    if(pendingSearches->remove(searchID) > 0)
    {
        killTimer(searchID);
    }
}

QStringList FileStore::getSharedFileNames()
{
    QStringList names;

    foreach(File file, *sharedFiles)
    {
        names.append(file.name());
    }

    return names;
}

bool FileStore::enDequeuePendingDownloadQueue()
{
    qDebug() << "ENDEQUEUE PENDING DOWNLOADQUEUE";
    
    Download* head = pendingDownloads->dequeue();
    while(!head->isAlive())
    {
        if(pendingDownloads->isEmpty())
        {
            return false;
        }
        head = pendingDownloads->dequeue();
    }

    qDebug() << "POP PENDING DOWNLOADQUEUE HEAD: "
             << head->toString();

    Message request;
    request.setType(TYPE_BLOCK_REQUEST);
    request.setDest(head->peer());
    request.setOriginID(ID);
    request.setHopLimit((quint32) BLOCK_HOP_LIMIT);

    if(head->needsMetaData())
    {
        QByteArray fileID = head->fileObject()->fileID();

        request.setBlockRequest(fileID);
        Q_EMIT(sendDirect(request, request.getDest()));

        head->begin();
        head->touch(fileID);

        Q_EMIT(updateDownloadInfo(*head));
        qDebug() << "SENT INITIAL METADATA REQUEST: " 
                 << request.toString();
    }
    else
    {
        QList<QByteArray> needed = head->blocksNeeded();
        QList<QByteArray>::iterator i;
        for(i = needed.begin(); i != needed.end(); ++i)
        {
            request.setBlockRequest(*i);
            Q_EMIT(sendDirect(request, request.getDest()));
            head->touch(*i);    // increment # times each request sent.

            qDebug() << "\tSENT BLOCK_REQUEST " << QString(i->toHex())
                     << " TO " << request.getOriginID();
        }
    }

    if(head->isAlive())
    {
        qDebug() << "RE-QUEUE'ING HEAD";

        pendingDownloads->enqueue(head);
        return true;
    }
    else
    {
        delete(head);
        return false;
    }
}

void FileStore::cyclePendingDownloadQueue()
{
    qDebug() << "BEGIN CYCLE PENDING DOWNLOADQUEUE";

    int size = pendingDownloads->size();
    bool headReQueued;

    for(int i = 0; i < size; i++)
    {
        qDebug() << "\tITERATION: " << QString::number(i);
        headReQueued = enDequeuePendingDownloadQueue();
        if(!headReQueued)
        {
            size--;
        }
    }

    qDebug() << "END CYCLE PENDING DOWNLOADQUEUE";
}

// ===========================FileStore::Search============================== //

FileStore::Search::Search(Message* m, int id)
    : msg(m)
    , nResults(0)
    , searchID(id)
    , keywordString(m->getSearch())
{
    msg->setType(TYPE_SEARCH_REQUEST);
}

FileStore::Search::~Search()
{}

void FileStore::Search::incrementBudget()
{
    msg->setBudget(msg->getBudget() + BUDGET_INCREMENT);
}

void FileStore::Search::incrementResults()
{
    nResults++;
}

int FileStore::Search::budget()
{
    return msg->getBudget();
}

int FileStore::Search::results()
{
    return nResults;
}

QString FileStore::Search::keywords()
{
    return keywordString;
}

Message FileStore::Search::message()
{
    return *msg;
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
    if(f->fileID().size() > 0)
    {
        insert(f->fileID(), 0);
    }
}

FileStore::Download::~Download()
{}

QString FileStore::Download::toString()
{
    QString qstr = "\n---------- Download ----------\n";
    qstr += "peer originID: " + peerID + "\n";
    qstr += "DownloadStatus: " + QString::number(downloadStatus) + "\n";
    qstr += "isAlive?: " + QString::number(isAlive()) + "\n";

    qstr += "touch counts:\n";
    foreach(QByteArray blockID, keys())
    {
        qstr += "\t[" + QString(blockID.toHex()) 
               + ": " + QString::number(value(blockID)) + "]\n";
    }

    qstr += "File:\n" + file->toString();
    qstr += "------------------------------\n\n";

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

    if(needsMetaData()) // need metadata!
    {
        needed.append(file->fileID());
        return needed;
    }

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
    return !file->hasMetaData();
}

bool FileStore::Download::needsBlock(QByteArray blockID)
{
    return blocksNeeded().contains(blockID);
}

bool FileStore::Download::isAlive()
{
    return (downloadStatus == DownloadStatus::INIT) ||
            ((downloadStatus == DownloadStatus::CONFIRMING) ||
            (downloadStatus == DownloadStatus::DOWNLOADING));
}

void FileStore::Download::begin()
{
    downloadStatus = DownloadStatus::CONFIRMING;
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
    if(!file->addMetaData(metaDataBytes))
    {
        qDebug() << "DOWNLOAD ALREADY HAS METADATA!";
        return;
    }

    remove(file->fileID());

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
        qDebug() << "bleeeehh!!";
        file->addBlock(blockID, blockData);
        if(file->isComplete())
        {
            complete();
        }
    }
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
{
    QList<Download*>::iterator i;
    for(i = this->begin(); i != this->end(); ++i)
    {
        delete(*i);
    }
    delete(this);
}

quint32 FileStore::DownloadQueue::size()
{
    return count();
}

FileStore::Download* FileStore::DownloadQueue::dequeue()
{
    return isEmpty() ? NULL : takeFirst();
}

void FileStore::DownloadQueue::enqueue(Download* d)
{
    append(d);
}

FileStore::Download* FileStore::DownloadQueue::search(QByteArray blockID)
{
    QList<Download*>::iterator i;
    for(i = this->begin(); i != this->end(); ++i)
    {
        if((*i)->fileObject()->containsBlock(blockID) ||
           (*i)->fileObject()->fileID() == blockID)
        {
            qDebug() << "FOUND MATCH FOR QUERY " << QString(blockID.toHex());
            return *i;
        }
    }
    qDebug() << "NO MATCH FOR QUERY " << QString(blockID.toHex());

    return NULL;    // blockID not found in any pending DL's blocks!
}

void FileStore::DownloadQueue::reap()
{
    if(!isEmpty())
    {
        QList<Download*>::iterator i = this->begin();
        while(i != this->end())
        {
            if(!(*i)->isAlive())
            {
                i = erase(i);
            }
            else
            {
                i++;
            }
        }
    }
}

bool FileStore::DownloadQueue::isEmpty()
{
    return count() == 0;
}





