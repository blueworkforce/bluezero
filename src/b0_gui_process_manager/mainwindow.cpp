#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
}

MainWindow::~MainWindow()
{
}

NodesView * MainWindow::nodesView() const
{
    return nodesView_;
}
