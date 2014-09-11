#ifndef PEERSTER_MESSAGE_HH
#define PEERSTER_MESSAGE_HH

#include "Peerster.hh"

class Message : public QVariantMap
{
    public:
        Message();
        Message(QByteArray*);
        ~Message();
        QString toString();
        QByteArray serialize();
        bool typeIsStatus();
        void setText(QString);
        void setOriginID(QString);
        void setSeqNo(quint32);
        void setPortOfOrigin(quint32);
        QString getText();
        QString getOriginID();
        quint32 getSeqNo();
        quint32 getPortOfOrigin();

    private:
        bool isStatus;
};

#endif // PEERSTER_MESSAGE_HH