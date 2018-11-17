#include "b0node.h"

#include <QDebug>

B0Node::B0Node()
{
}

void B0Node::onGraphChanged(const b0::message::graph::Graph &msg)
{
    qDebug() << "onGraphChanged" << msg.nodes.size();

    QMap<QString, QString> node_topic, topic_node, node_service, service_node;
    for(auto x : msg.node_topic)
    {
        QString node(QString::fromStdString(x.node_name));
        QString topic(QString::fromStdString(x.other_name));
        if(x.reversed)
            topic_node[topic] = node;
        else
            node_topic[node] = topic;
    }
    for(auto x : msg.node_service)
    {
        QString node(QString::fromStdString(x.node_name));
        QString service(QString::fromStdString(x.other_name));
        if(x.reversed)
            service_node[service] = node;
        else
            node_service[node] = service;
    }
    Q_EMIT graphChanged(node_topic, topic_node, node_service, service_node);
}

void B0Node::onActiveNodesChanged(const b0::process_manager::ActiveNodes &msg)
{
    qDebug() << "onActiveNodesChanged" << msg.nodes.size();

    QStringList activeNodes;
    for(auto x : msg.nodes)
        activeNodes << QString::fromStdString(x.host_name);
    Q_EMIT activeNodesChanged(activeNodes);
}

void B0Node::run(int argc, char **argv)
{
    b0::init(argc, argv);

    node_.reset(new b0::Node("b0_gui_process_manager"));
    graph_sub_.reset(new b0::Subscriber(node_.get(), "graph", static_cast<b0::Subscriber::CallbackMsg<b0::message::graph::Graph> >(boost::bind(&B0Node::onGraphChanged, this, _1))));
    active_nodes_sub_.reset(new b0::Subscriber(node_.get(), "process_manager_hub/active_nodes", static_cast<b0::Subscriber::CallbackMsg<b0::process_manager::ActiveNodes> >(boost::bind(&B0Node::onActiveNodesChanged, this, _1))));
    pm_cli_.reset(new b0::ServiceClient(node_.get(), "process_manager_hub/control"));

    node_->init();
    node_->spin();
    node_->cleanup();

    Q_EMIT finished();
}

void B0Node::startNode(const QString &host, const QString &program, const QStringList &args, bool *success, int *pid, QString *error)
{
    b0::process_manager::Request req;
    req.host_name = host.toStdString();
    req.start_process.emplace();
    req.start_process->path = program.toStdString();
    for(auto arg : args)
        req.start_process->args.push_back(arg.toStdString());
    b0::process_manager::Response resp;
    pm_cli_->call(req, resp);
    *success = resp.start_process->success;
    if(*success)
        *pid = *resp.start_process->pid;
    else if(resp.start_process->error_message)
        *error = QString::fromStdString(*resp.start_process->error_message);
}

void B0Node::stopNode(const QString &host, int pid, bool *success, QString *error)
{
    b0::process_manager::Request req;
    req.host_name = host.toStdString();
    req.stop_process.emplace();
    req.stop_process->pid = pid;
    b0::process_manager::Response resp;
    pm_cli_->call(req, resp);
    *success = resp.stop_process->success;
    if(!*success && resp.stop_process->error_message)
        *error = QString::fromStdString(*resp.stop_process->error_message);
}
