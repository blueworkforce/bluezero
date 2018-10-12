#include <b0/b0.h>
#include <b0/utils/env.h>
#include <b0/node.h>
#include <b0/logger/logger.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>

namespace b0
{

namespace po = boost::program_options;

struct Global::Private
{
    bool initialized_ = false;
    std::map<std::string, std::string> remap_node_;
    std::map<std::string, std::string> remap_topic_;
    std::map<std::string, std::string> remap_service_;
    logger::Level consoleLogLevel_ = logger::Level::info;
};

Global::Global()
    : private_(new Private),
      options_description_("Allowed options")
{
    options_description_.add_options()
        ("help,h", "display help message")
        ("remap,R", po::value<std::string>()->value_name("oldName=newName")->notifier(boost::bind(&Global::addRemap, this, _1)), "remap any name")
        ("remap-node,N", po::value<std::string>()->value_name("oldName=newName")->notifier(boost::bind(&Global::addNodeRemap, this, _1)), "remap a node name")
        ("remap-topic,T", po::value<std::string>()->value_name("oldName=newName")->notifier(boost::bind(&Global::addTopicRemap, this, _1)), "remap a topic name")
        ("remap-service,S", po::value<std::string>()->value_name("oldName=newName")->notifier(boost::bind(&Global::addServiceRemap, this, _1)), "remap a service name")
        ("console-loglevel,L", po::value<std::string>()->default_value("info"), "specify the console loglevel")
    ;
}

Global & Global::getInstance()
{
    static Global global;
    return global;
}

static std::vector<std::string> splitAssignment(const std::string &raw_arg)
{
    std::vector<std::string> ret;
    boost::split(ret, raw_arg, boost::is_any_of("="));
    if(ret.size() != 2)
        throw std::runtime_error("argument must be origName=newName");
    return ret;
}

void Global::addRemap(const std::string &raw_arg)
{
    auto x = splitAssignment(raw_arg);
    addRemap(x[0], x[1]);
}

void Global::addRemap(const std::string &orig_name, const std::string &new_name)
{
    addNodeRemap(orig_name, new_name);
    addTopicRemap(orig_name, new_name);
    addServiceRemap(orig_name, new_name);
}

void Global::addNodeRemap(const std::string &raw_arg)
{
    auto x = splitAssignment(raw_arg);
    addNodeRemap(x[0], x[1]);
}

void Global::addNodeRemap(const std::string &orig_name, const std::string &new_name)
{
    private_->remap_node_[orig_name] = new_name;
}

void Global::addTopicRemap(const std::string &raw_arg)
{
    auto x = splitAssignment(raw_arg);
    addTopicRemap(x[0], x[1]);
}

void Global::addTopicRemap(const std::string &orig_name, const std::string &new_name)
{
    private_->remap_topic_[orig_name] = new_name;
}

void Global::addServiceRemap(const std::string &raw_arg)
{
    auto x = splitAssignment(raw_arg);
    addServiceRemap(x[0], x[1]);
}

void Global::addServiceRemap(const std::string &orig_name, const std::string &new_name)
{
    private_->remap_service_[orig_name] = new_name;
}

po::options_description & Global::optionsDescription()
{
    return options_description_;
}

po::positional_options_description & Global::positionalOptionsDescription()
{
    return positional_options_description_;
}

po::variables_map & Global::options()
{
    return variables_map_;
}

void Global::printUsage(bool toStdErr)
{
    (toStdErr ? std::cerr : std::cout) << options_description_ << std::endl;
}

void Global::init(int &argc, char **argv)
{
    if(private_->initialized_)
        throw std::runtime_error("already initialized");

    // process environment variables:
    std::string console_loglevel = b0::env::get("B0_CONSOLE_LOGLEVEL");
    if(console_loglevel != "")
    {
        private_->consoleLogLevel_ = logger::levelInfo(console_loglevel).level;
    }

    // process arguments:
    try
    {
        po::command_line_parser parser(argc, argv);
        parser.options(options_description_).positional(positional_options_description_);
        po::parsed_options parsed_options = parser.run();
        po::store(parsed_options, variables_map_);
        po::notify(variables_map_);
    }
    catch(po::error &ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        printUsage(true);
        std::exit(1);
    }

    if(variables_map_.count("help"))
    {
        printUsage();
        std::exit(0);
    }

    if(variables_map_.count("console-loglevel"))
    {
        private_->consoleLogLevel_ = logger::levelInfo(variables_map_["console-loglevel"].as<std::string>()).level;
    }

    private_->initialized_ = true;
}

bool Global::isInitialized() const
{
    return private_->initialized_;
}

static bool makeSubstitutions(const b0::Node &node, std::string &name)
{
    bool ret = false;

    if(name.find("%h") != std::string::npos)
    {
        boost::replace_all(name, "%h", node.hostname());
        ret = true;
    }

    if(name.find("%n") != std::string::npos)
    {
        boost::replace_all(name, "%n", node.getName());
        ret = true;
    }

    return ret;
}

static bool remapName(const b0::Node &node, const std::map<std::string, std::string> &map, const std::string &name, std::string &remapped_name)
{
    auto i = map.find(name);
    bool ret = false;
    if(i == map.end())
    {
        remapped_name = name;
    }
    else
    {
        remapped_name = i->second;
        ret = true;
    }
    return makeSubstitutions(node, remapped_name) || ret;
}

static std::string getRemappedName(const b0::Node &node, const std::map<std::string, std::string> &map, const std::string &name)
{
    std::string ret = name;
    remapName(node, map, name, ret);
    return ret;
}

std::string Global::getRemappedNodeName(const b0::Node &node, const std::string &node_name)
{
    return getRemappedName(node, private_->remap_node_, node_name);
}

std::string Global::getRemappedTopicName(const b0::Node &node, const std::string &topic_name)
{
    return getRemappedName(node, private_->remap_topic_, topic_name);
}

std::string Global::getRemappedServiceName(const b0::Node &node, const std::string &service_name)
{
    return getRemappedName(node, private_->remap_service_, service_name);
}

bool Global::remapNodeName(const b0::Node &node, const std::string &node_name, std::string &remapped_node_name)
{
    return remapName(node, private_->remap_node_, node_name, remapped_node_name);
}

bool Global::remapTopicName(const b0::Node &node, const std::string &topic_name, std::string &remapped_topic_name)
{
    return remapName(node, private_->remap_topic_, topic_name, remapped_topic_name);
}

bool Global::remapServiceName(const b0::Node &node, const std::string &service_name, std::string &remapped_service_name)
{
    return remapName(node, private_->remap_service_, service_name, remapped_service_name);
}

logger::Level Global::getConsoleLogLevel()
{
    return private_->consoleLogLevel_;
}

void Global::setConsoleLogLevel(logger::Level level)
{
    private_->consoleLogLevel_ = level;
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

void printUsage(bool toStdErr)
{
    Global::getInstance().printUsage(toStdErr);
}

boost::program_options::options_description_easy_init addOptions()
{
    return Global::getInstance().optionsDescription().add_options();
}

void addPositionalOption(const std::string &option, int max_count)
{
    Global::getInstance().positionalOptionsDescription().add(option.c_str(), max_count);
}

int hasOption(const std::string &option)
{
    return Global::getInstance().options().count(option);
}

const boost::program_options::variable_value & getOption(const std::string &option)
{
    return Global::getInstance().options()[option];
}

logger::Level getConsoleLogLevel()
{
    return Global::getInstance().getConsoleLogLevel();
}

void setConsoleLogLevel(logger::Level level)
{
    Global::getInstance().setConsoleLogLevel(level);
}

using boost::program_options::value;

} // namespace b0

