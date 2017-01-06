#include "widget.h"
#include <cassert>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    ScannerMain window;
    window.show();

    return application.exec();
}
