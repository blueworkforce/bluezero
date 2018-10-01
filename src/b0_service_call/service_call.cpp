#include <string>
#include <iostream>
#include <iterator>

#include <boost/program_options.hpp>

#include <b0/node.h>
#include <b0/service_client.h>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    b0::init(argc, argv);
    std::string node_name = "b0_service_call", service_name = "", content_type = "";
    double rate = 0.0;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display help message")
        ("node-name,n", po::value<std::string>(&node_name), "name of node")
        ("service-name,s", po::value<std::string>(&service_name), "name of service")
        ("content-type,c", po::value<std::string>(&content_type), "content type")
    ;
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(po::unknown_option &ex)
    {
        std::cerr << ex.what() << std::endl << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    std::cin >> std::noskipws;
    std::istream_iterator<char> it(std::cin);
    std::istream_iterator<char> end;
    std::string request(it, end), response, response_type;

    b0::Node node(node_name);
    b0::ServiceClient cli(&node, service_name);
    node.init();
    cli.call(request, content_type, response, response_type);
    if(content_type != "")
        std::cout << "Content-type: " << response_type << std::endl;
    std::cout << response << std::flush;
    node.cleanup();
    return 0;
}

