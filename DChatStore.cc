#include "DChatStore.hh"

DChatStore::DChatStore(Peerster* p)
    : peerster(p)
    , histories(new QMap< QString, QList<Message> >())
{}

DChatStore::~DChatStore()
{}

void DChatStore::addDChat(Message dmsg)
{
    QString origin = (dmsg.getOriginID() != ID) ? dmsg.getOriginID() 
                                                : dmsg.getDest(); 
    QList<Message> history;
    if(histories->contains(origin))
    {
        history = histories->value(origin);
        history.append(dmsg);  
        histories->insert(origin, history); 
    }
    else
    {
        history = QList<Message>();
        history.append(dmsg);
    }

    Q_EMIT(updateGUIDChatHistory(origin, histories->value(origin)));
}

void DChatStore::gotGetDChatHistoryFromOrigin(QString origin)
{
    QList<Message> history;
    if(histories->contains(origin))
    {
        history = histories->value(origin);
    }
    else
    {
        history = QList<Message>();
    }

    Q_EMIT(updateGUIDChatHistory(origin, history));
}

void DChatStore::setID(QString qstr)
{
    ID = qstr;
}