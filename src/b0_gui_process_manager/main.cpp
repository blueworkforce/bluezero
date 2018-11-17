#include "mainwindow.h"
#include "b0node.h"
#include <QApplication>
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
    QObject::connect(node, &B0Node::activeNodesChanged, [=](QSet<QString> activeNodes) {
        //qDebug() << "activeNodesChanged...";
        //qDebug() << activeNodes;
    });
    QObject::connect(node, &B0Node::graphChanged, w.nodesView_, &NodesView::setGraph);
    w.show();

    return a.exec();
}
