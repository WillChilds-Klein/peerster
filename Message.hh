#ifndef PEERSTER_MESSAGE_HH
#define PEERSTER_MESSAGE_HH

#include "Peerster.hh"

class Message : public QVariantMap
{
    public:
        Message();
        Message(QByteArray* arr);
        ~Message();
        QString toString();
        QByteArray serialize();
        bool typeIsStatus();
        void setText(QString);
        void setOriginID(QString);
        void setSeqNo(quint32);

    private:
        bool isStatus;
};

#endif // PEERSTER_MESSAGE_HH