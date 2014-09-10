#ifndef PEERSTER_MAILBOX_HH
#define PEERSTER_MAILBOX_HH

#include "Peerster.hh"

class Peerster;
class Message;

class Mailbox : public QObject
{
    Q_OBJECT

    public:
        Mailbox(Peerster* p);
        ~Mailbox();

    public slots:
        void gotInboxUpdated();
        void gotOutboxUpdated();

    private:
        Peerster* peerster;
};

#endif // PEERSTER_MAILBOX_HH

