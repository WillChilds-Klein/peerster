#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "Peerster.hh"

Peerster::Peerster()
    : dialog(new ChatDialog(this))
    , socket(new NetSocket(this))
{}

Peerster::~Peerster() {}

ChatDialog* Peerster::getDialog() 
{
    return this->dialog;
}

NetSocket* Peerster::getSocket()
{
    return this->socket;
}