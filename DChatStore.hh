#ifndef PEERSTER_DCHAT_STORE
#define PEERSTER_DCHAT_STORE

#include "Peerster.hh"

class DChatStore : public QObject
{
    Q_OBJECT

    public:
        DChatStore();
        ~DChatStore();
        void addDChat(Message);
        void getDChatHistoryFromOrigin(QString);

    signals:
        void updateGUIDChatHistory(QString,QList<Message>);

    private:
        Peerster* peerster;
        QMap< QString, QList<Message> >* histories; 
};

#endif // PEERSTER_DCHATSTORE_HH