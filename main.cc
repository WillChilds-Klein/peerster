#include "Peerster.hh"

int main(int argc, char** argv)
{
    // Initialize Qt toolkit
    QApplication app(argc,argv);

    QCA::Initializer qcainit;

    Peerster peerster;
    peerster.run();

    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}

