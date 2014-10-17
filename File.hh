#ifndef PEERSTER_FILE_HH
#define PEERSTER_FILE_HH

#include "Peerster.hh"

class File : public QVariantMap
{
    public:
        File();
        File(QString);
        ~File();
        QString name();
        quint32 size();

    private:
        QString fileName;
        quint32 fileSize;
};

#endif // PEERSTER_FILE_HH