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
        QByteArray toSerializedQVMap();
        bool isWellFormed();
        bool isEmptyStatus();
        bool isEmptySearchRequest();
        bool isDirectRumor();
        bool isValidBlockReply();
        bool isValidSearchReply();
        void setType(QString);
        void setChatText(QString);
        void setOriginID(QString);
        void setSeqNo(quint32);
        void setWantMap(QVariantMap);
        void setDest(QString);
        void setHopLimit(quint32);
        void setLastIP(quint32);
        void setLastPort(quint16);
        void setBlockRequest(QByteArray);
        void setBlockReply(QByteArray);
        void setData(QByteArray);
        void setSearch(QString);
        void setBudget(quint32);
        void setSearchReply(QString);
        void setMatchNames(QVariantList);
        void setMatchIDs(QByteArray);
        QString getType();
        QString getText();
        QString getOriginID();
        quint32 getSeqNo();
        QString getDest();
        quint32 getHopLimit();
        QVariantMap getWantMap();
        quint32 getLastIP();
        quint16 getLastPort();
        QByteArray getBlockRequest();
        QByteArray getBlockReply();
        QByteArray getData();
        QString getSearch();
        quint32 getBudget();
        QString getSearchReply();
        QVariantList getMatchNames();
        QByteArray getMatchIDs();

    private:
        bool wellFormed;
        void setWellFormed(bool);
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