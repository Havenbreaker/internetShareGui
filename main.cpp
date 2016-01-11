#include "internetshare.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    InternetShare w;
    w.show();
    return a.exec();
}
