#include "tinyweather_mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    tinyweather_mainWindow w;
    w.show();

    return a.exec();
}
