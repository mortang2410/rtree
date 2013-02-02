#include "mymainwindow.h"
#include "ui_mymainwindow.h"

MyMainWindow::MyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyMainWindow)
{
    ui->setupUi(this);
    rectGroup = new QActionGroup(this);
    rectGroup->addAction(ui->actionAddRectangle);
    rectGroup->addAction(ui->actionRemoveRectangle);
    rectGroup->addAction(ui->actionSearchRect);
    ui->actionAddRectangle->setChecked(true);
    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    ui->rectview->scene->treescene =  ui->rtreeview->scene;

    connect(ui->actionAddRectangle
            ,SIGNAL(toggled(bool)),this,SLOT(addMode(bool)));
    connect(ui->actionRemoveRectangle,SIGNAL(toggled(bool)),
            this,SLOT(removeMode(bool)));
    connect(ui->actionSearchRect,SIGNAL(toggled(bool)),
            this,SLOT(searchMode(bool)));
}

MyMainWindow::~MyMainWindow()
{
    delete ui;
}

void MyMainWindow::addMode(bool a)
{
    if (a) ui->rectview->scene->setMode(RectScene::Add);
}

void MyMainWindow::removeMode(bool a)
{
    if (a) ui->rectview->scene->setMode(RectScene::Remove);
}

void MyMainWindow::searchMode(bool a)
{
    if (a) ui->rectview->scene->setMode(RectScene::Search);
}
