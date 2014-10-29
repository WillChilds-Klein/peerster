#include "Peerster.hh"

int main(int argc, char** argv)
{
    QCA::Initializer qcainit;
 
    // Initialize Qt toolkit
    QApplication app(argc,argv);

    Peerster peerster;
    peerster.run();

    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}

