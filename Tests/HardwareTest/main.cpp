#include <QtGui/QApplication>
#include <QThreadPool>
#include "hardwaretest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QThreadPool::globalInstance()->setMaxThreadCount(4);
    HardwareTest w;
    system("BackLight initrobot");
    w.show();
    
    return a.exec();
}
