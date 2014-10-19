#include "FileStore.hh"

#include SHARED_FILE_DIR_PREFIX ("/tmp/peerster-")

FileStore::FileStore(Peerster* p)
    : peerster(p)
{
    quint32 modulus = qPow(10,TEMPDIR_NDIGITS);
    tempdir = new QDir(QDir::tempPath() + "/peerster-" 
                     + QString::number((qrand() % modulus) + 1));
}

FileStore::~FileStore()
{}

void FileStore::setID(QString id)
{
    ID = id;
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
            File file = File(filepath, tempdir.path());
            sharedFiles->append(file));
            sharedFileInfo->insert(file.abspath(), file.size());
        }
    }

    Q_EMIT(refreshSharedFiles());
}
