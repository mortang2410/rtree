#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H
#include "views.h"

namespace Ui {
class MyMainWindow;
}

class MyMainWindow : public QMainWindow
{
    Q_OBJECT    
public:
    explicit MyMainWindow(QWidget *parent = 0);
    ~MyMainWindow();    
public slots:
    void addMode(bool a);
    void removeMode(bool a);
    void searchMode(bool a);
private:
    Ui::MyMainWindow *ui;


    QActionGroup * rectGroup;
};

#endif // MYMAINWINDOW_H
