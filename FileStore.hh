#ifndef PEERSTER_FILESTORE_HH
#define PEERSTER_FILESTORE_HH

#include "Peerster.hh"

class Message;

class FileStore : public QObject
{
    Q_OBJECT

    public:
        FileStore(Peerster*);
        ~FileStore();
        void setID(QString);
        void setSharedFileInfo(QMap<QString,quint32>*);

    // public slots:
    //     void gotProcessShareFiles(QStringList);

    private:
        Peerster* peerster;
        QString ID;
        QMap<QString, quint32>* sharedFileInfo;
};

#endif // PEERSTER_FILESTORE_HH