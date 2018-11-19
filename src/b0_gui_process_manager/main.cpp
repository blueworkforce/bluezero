#include "mainwindow.h"
#include "b0node.h"
#include <QApplication>
#include <QThread>
#include <QDebug>

class B0NodeWorker : public QObject
{
    Q_OBJECT

public:
    B0NodeWorker(QThread *thread)
    {
        B0Node *node = &node_;
        int argc = 1;
        char *argv[] = {"foo"};
        QObject::connect(thread, &QThread::started, [=]() {node->run(argc, (char**)argv);});
        QObject::connect(node, &B0Node::finished, thread, &QThread::quit);
        QObject::connect(node, &B0Node::finished, node, &B0Node::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    }

    B0Node node_;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<QSet<QString> >("QSet<QString>");
    qRegisterMetaType<QMap<QString,QString> >("QMap<QString,QString>");

    QThread *thread = new QThread();
    B0NodeWorker *worker = new B0NodeWorker(thread);
    worker->moveToThread(thread);
    thread->start();

    MainWindow mainWindow(&worker->node_);
    mainWindow.show();

    return a.exec();
}
