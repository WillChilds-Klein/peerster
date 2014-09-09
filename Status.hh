#ifndef PEERSTER_STATUS_HH
#define PEERSTER_STATUS_HH

#include "Peerster.hh"

class Status : public Message
{
    public:
        Status();
        ~Status();
};

#endif // PEERSTER_STATUS_HH