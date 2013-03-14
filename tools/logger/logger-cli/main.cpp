#include <QCoreApplication>

#include "clilogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    CliLogger logger;

    return a.exec();
}
