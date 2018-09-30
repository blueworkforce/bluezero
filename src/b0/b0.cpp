#include <b0/b0.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include <boost/format.hpp>

namespace b0
{

struct Global::Private
{
    bool initialized_ = false;
    std::map<std::string, std::string> remap_node_;
    std::map<std::string, std::string> remap_topic_;
    std::map<std::string, std::string> remap_service_;
};

Global::Global()
    : private_(new Private)
{
}

Global & Global::getInstance()
{
    static Global global;
    return global;
}

void Global::init(int &argc, char **argv)
{
    if(private_->initialized_)
        throw std::runtime_error("already initialized");

    // process arguments:
    int i = 0;
    std::vector<int> processed_args;
    while(++i < argc)
    {
        std::string opt(argv[i]);
        if((i + 1) < argc)
        {
            // process options with 1 argument here:
            std::string arg(argv[i + 1]);
            if(opt == "--remap" || opt == "-R" || opt == "--remap-node" || opt == "-Rn" || opt == "--remap-topic" || opt == "-Rt" || opt == "--remap-service" || opt == "-Rs")
            {
                const std::string sep = "=";
                size_t pos = arg.find(sep);
                if(pos == std::string::npos)
                    throw std::runtime_error((boost::format("%s argument must be a pair of names separated by '%s', e.g.: localname%sexternalname (got: %s)") % opt % sep % sep % arg).str());
                std::string local_name = arg.substr(0, pos),
                    external_name = arg.substr(pos + sep.size());
                if(opt == "--remap" || opt == "-R" || opt == "--remap-node" || opt == "-Rn")
                    private_->remap_node_[local_name] = external_name;
                if(opt == "--remap" || opt == "-R" || opt == "--remap-topic" || opt == "-Rt")
                    private_->remap_topic_[local_name] = external_name;
                if(opt == "--remap" || opt == "-R" || opt == "--remap-service" || opt == "-Rs")
                    private_->remap_service_[local_name] = external_name;
                processed_args.push_back(i);
                processed_args.push_back(i + 1);
            }
        }
    }

    // mnove processed arguments to end:
    int shift = 0;
    for(int i : processed_args)
    {
        char *tmp = argv[i + shift];
        for(int j = i + shift + 1; j < argc; j++)
            argv[j - 1] = argv[j];
        argv[argc - 1] = tmp;
        shift--;
    }
    // hide processed arguments:
    argc -= processed_args.size();

    private_->initialized_ = true;
}

bool Global::isInitialized() const
{
    return private_->initialized_;
}

static std::string getRemappedName(const std::map<std::string, std::string> &map, const std::string &name)
{
    auto i = map.find(name);
    return i == map.end() ? name : i->second;
}

static bool remapName(const std::map<std::string, std::string> &map, const std::string &name, std::string &remapped_name)
{
    auto i = map.find(name);
    if(i == map.end())
    {
        remapped_name = name;
        return false;
    }
    else
    {
        remapped_name = i->second;
        return true;
    }
}

std::string Global::getRemappedNodeName(const std::string &node_name)
{
    return getRemappedName(private_->remap_node_, node_name);
}

std::string Global::getRemappedTopicName(const std::string &topic_name)
{
    return getRemappedName(private_->remap_topic_, topic_name);
}

std::string Global::getRemappedServiceName(const std::string &service_name)
{
    return getRemappedName(private_->remap_service_, service_name);
}

bool Global::remapNodeName(const std::string &node_name, std::string &remapped_node_name)
{
    return remapName(private_->remap_node_, node_name, remapped_node_name);
}

bool Global::remapTopicName(const std::string &topic_name, std::string &remapped_topic_name)
{
    return remapName(private_->remap_topic_, topic_name, remapped_topic_name);
}

bool Global::remapServiceName(const std::string &service_name, std::string &remapped_service_name)
{
    return remapName(private_->remap_service_, service_name, remapped_service_name);
}

void init(int &argc, char **argv)
{
    try
    {
        Global::getInstance().init(argc, argv);
    }
    catch(std::exception &ex)
    {
        std::cerr << "Initialization failed: " << ex.what() << std::endl;
        std::exit(100);
    }
}

} // namespace b0

