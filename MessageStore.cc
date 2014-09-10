#include "MessageStore.hh"

MessageStore::MessageStore()
{}

MessageStore::~MessageStore()
{}

bool MessageStore::isNewMessage(Message msg)
{
    if(msg.type == Message::STATUS)
        return false;
    return true;
}