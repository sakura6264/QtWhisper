#include "mainwindow.h"
#include "phantomstyle.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PhantomStyle *s = new PhantomStyle;
    a.setStyle(s);
    MainWindow w;
    w.show();
    int ret = a.exec();
    return ret;
}
