#include <b0/b0.h>

class Node
{
public:
    Node(const std::string &name)
        : node_(new b0::Node(name))
    {
    }

    ~Node()
    {
        delete node_;
    }

    void init()
    {
        node_->init();
    }

    void cleanup()
    {
        node_->cleanup();
    }

private:
    b0::Node *node_;

    friend class Publisher;
};

class Publisher
{
public:
    Publisher(Node &node, const std::string &topic)
        : pub_(new b0::Publisher(node.node_, topic))
    {
    }

    ~Publisher()
    {
        delete pub_;
    }

    void publish(const std::string &data)
    {
        pub_->publish(data);
    }

private:
    b0::Publisher *pub_;
};

