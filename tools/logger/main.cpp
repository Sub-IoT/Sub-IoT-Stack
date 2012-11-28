#include <QApplication>
#include "loggerdialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoggerDialog w;
    w.show();
    
    return a.exec();
}
