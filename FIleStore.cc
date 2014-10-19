#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
    , sharedFiles(new QList<File>)
{}

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