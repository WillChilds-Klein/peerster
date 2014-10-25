#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
    , requestTimer(new QTimer(this))
{
    downloads = new QDir(QDir::currentPath() + DOWNLOADS_DIR_NAME);
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
            File file = File(filepath, tempdir->path());
            sharedFiles->append(file);
            sharedFileInfo->insert(file.abspath(), file.size());
        }
    }

    Q_EMIT(refreshSharedFiles());
}

void FileStore::gotRequestFile(QString origin, QString hash)
{
    Message msg;

    msg.setType(TYPE_BLOCK_REQUEST);
    msg.setDest(origin);
    msg.setOriginID(ID);
    msg.setHopLimit((quint32) BLOCK_HOP_LIMIT);
    msg.setBlockRequest(QByteArray::fromHex(hash.toLatin1()));

    Q_EMIT(sendDirect(msg, origin));

    qDebug() << "SENT " << msg.toString();
    qDebug() << "TO: " << origin;
}

void FileStore::gotProcessBlockRequest(Message msg)
{
    QByteArray data = msg.getBlockRequest();
    Message reply;
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
            reply.setData(file.metafile());

            Q_EMIT(sendDirect(reply.getDest(), reply));
            return;
        }
        // else scan individual file blocks
        else if(file.containsBlockID(data))
        {
            reply.setOriginID(ID);
            reply.setDest(msg.getOriginID());
            reply.setHopLimit(BLOCK_HOP_LIMIT);
            reply.setBlockReply(data);
            reply.setData(file.block(data));

            Q_EMIT(sendDirect(reply.getDest(), reply));
            return;
        }
    }

}

void FileStore::gotProcessBlockReply(Message msg)
{
    // TODO
}

void FileStore::gotProcessSearchRequest(Message msg)
{
    // TODO
}

void FileStore::gotProcessSearchReply(Message msg)
{
    // TODO
}

void FileStore::makeTempdir()
{
    if(tempdir)
    {
        tempdir = new QDir(QDir::tempPath() + "/peerster-" + ID);
        
        if(!tempdir->exists())
        {
            tempdir->mkpath(tempdir->path());
            qDebug() << "SUCESSFULLY CREATED TEMP DIR " << tempdir->path();
        }
    }
}