#include "mainwindow.h"
#include "b0node.h"
#include <QApplication>
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QThread *thread = new QThread();
    B0Node *node = new B0Node();
    node->moveToThread(thread);
    QObject::connect(thread, &QThread::started, [=]() {node->run(argc, argv);});
    QObject::connect(node, &B0Node::finished, thread, &QThread::quit);
    QObject::connect(node, &B0Node::finished, node, &B0Node::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();

    MainWindow w;
    QObject::connect(node, &B0Node::graphChanged, [=](QMap<QString, QString> node_topic, QMap<QString, QString> topic_node, QMap<QString, QString> node_service, QMap<QString, QString> service_node) {
        qDebug() << "graphChanged...";
        qDebug() << node_topic;
        qDebug() << topic_node;
        qDebug() << node_service;
        qDebug() << service_node;
    });
    w.show();

    return a.exec();
}
