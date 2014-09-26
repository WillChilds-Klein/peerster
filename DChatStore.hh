#ifndef PEERSTER_DCHAT_STORE
#define PEERSTER_DCHAT_STORE

#include "Peerster.hh"

class Peerster;

class DChatStore : public QObject
{
    Q_OBJECT

    public:
        DChatStore(Peerster*);
        ~DChatStore();
        void newDChat(Message);
        void setID(QString);

    signals:
        void updateGUIDChatHistory(QString,QList<Message>);

    public slots:
        void gotGetDChatHistoryFromOrigin(QString);

    private:
        Peerster* peerster;
        QString ID;
        QMap< QString, QList<Message> >* histories; 
};

#endif // PEERSTER_DCHATSTORE_HH