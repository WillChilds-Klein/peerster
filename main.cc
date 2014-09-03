#include "Peerster.hh"
#include "ChatDialog.hh"
#include "NetSocket.hh"
#include "Message.hh"

int main(int argc, char** argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	Peerster peerster;

	// Create an initial chat dialog window
	ChatDialog* dialog = peerster.getDialog();
	dialog->show();

	// Create a UDP network socket
	NetSocket* sock = peerster.getSocket();
	if (!sock->bind())
		exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

