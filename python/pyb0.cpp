#include <boost/python.hpp>

#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>

using namespace boost::python;

void Node_spin(b0::Node *node)
{
    node->spin();
}

b0::Subscriber * Subscriber_new(b0::Node *node, std::string topic_name, object const &callback)
{
    return new b0::Subscriber(node, topic_name, [=](std::string payload) {callback(payload);});
}

BOOST_PYTHON_MODULE(pyb0)
{
    class_<b0::Node, boost::noncopyable>
        ("Node", init<std::string>())
        .def("init", &b0::Node::init)
        .def("cleanup", &b0::Node::cleanup)
        .def("spin_once", &b0::Node::spinOnce)
        .def("spin", &Node_spin)
    ;
    class_<b0::Publisher, boost::noncopyable>
        ("Publisher", init<b0::Node*, std::string>())
        .def("init", &b0::Publisher::init)
        .def("cleanup", &b0::Publisher::cleanup)
        .def("get_topic_name", &b0::Publisher::getTopicName)
        .def("publish", &b0::Publisher::publish)
    ;
    class_<b0::Subscriber, boost::noncopyable>
        ("Subscriber", no_init)
        .def("__init__", make_constructor(&Subscriber_new))
        .def("init", &b0::Subscriber::init)
        .def("cleanup", &b0::Subscriber::cleanup)
        .def("get_topic_name", &b0::Subscriber::getTopicName)
    ;
}

