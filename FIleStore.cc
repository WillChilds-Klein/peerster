#include "FileStore.hh"

FileStore::FileStore(Peerster* p)
    : peerster(p)
{}

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


