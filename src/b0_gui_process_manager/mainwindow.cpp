#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(B0Node *node, QWidget *parent)
    : QMainWindow(parent),
      node_(node)
{
    setupUi(this);

    connect(node_, &B0Node::activeNodesChanged, nodesView_, &NodesView::setActiveNodes, Qt::QueuedConnection);
    connect(node_, &B0Node::graphChanged, nodesView_, &NodesView::setGraph, Qt::QueuedConnection);
    connect(nodesView_->startNodeDialog_, &StartNodeDialog::startNode, node_, &B0Node::startNode, Qt::QueuedConnection);
    connect(node_, &B0Node::startNodeResult, nodesView_->startNodeDialog_, &StartNodeDialog::displayStartNodeResult, Qt::QueuedConnection);
    connect(nodesView_, &NodesView::stopNode, node_, &B0Node::stopNode, Qt::QueuedConnection);
    connect(node_, &B0Node::stopNodeResult, [=](bool ok, const  QString &error_message) {
        if(!ok)
            QMessageBox::critical(this, "Error", "Failed to stop node");
    });
}

MainWindow::~MainWindow()
{
}

NodesView * MainWindow::nodesView() const
{
    return nodesView_;
}
