#include "mymainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyMainWindow window;
    window.show();
    return a.exec();
}
