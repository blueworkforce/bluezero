#include "mainwindow.h"
#include "b0node.h"
#include <QApplication>
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<QSet<QString> >("QSet<QString>");
    qRegisterMetaType<QMap<QString,QString> >("QMap<QString,QString>");

    QThread *thread = new QThread();
    B0Node *node = new B0Node();
    node->moveToThread(thread);
    QObject::connect(thread, &QThread::started, [=]() {node->run(argc, argv);});
    QObject::connect(node, &B0Node::finished, thread, &QThread::quit);
    QObject::connect(node, &B0Node::finished, node, &B0Node::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();

    MainWindow w;
    QObject::connect(node, &B0Node::activeNodesChanged, w.nodesView_, &NodesView::setActiveNodes);
    QObject::connect(node, &B0Node::graphChanged, w.nodesView_, &NodesView::setGraph);
    w.show();

    return a.exec();
}
