#include "LibmpvPlayer.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    LibmpvPlayer player;
    player.show();
    return app.exec();
}
