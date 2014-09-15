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
        void setIsRumor(bool);
        void setText(QString);
        void setOriginID(QString);
        void setSeqNo(quint32);
        void setPortOfOrigin(quint32);
        void setWantMap(QVariantMap);
        bool typeIsRumor();
        QString getText();
        QString getOriginID();
        quint32 getSeqNo();
        quint32 getPortOfOrigin();
        QVariantMap getWantMap();

    private:
        bool isRumor;
};

// TODO: implement Rumor/Status sub-types of Message in all code.

// class Message : public QVariantMap
// {
//     public:
//         Message();
//         Message(QByteArray*);
//         ~Message();
//         QString toString();
//         QByteArray serialize();
//         void setPortOfOrigin(quint32);
//         quint32 getPortOfOrigin();
// };

// class Rumor : Message
// {
//     public:
//         void setText(QString);
//         void setOriginID(QString);
//         void setSeqNo(quint32);
//         QString getText();
//         QString getOriginID();
//         quint32 getSeqNo();
// };

// class Status : Message
// {
//     public:
//         void setWantMap(QVariantMap);
//         QVariantMap getWantMap();
// };

#endif // PEERSTER_MESSAGE_HH