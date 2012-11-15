#include <iostream>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>

#include "test_strategy.hpp"
#include "transaq_client.hpp"

boost::program_options::options_description available_options()
{
    static boost::program_options::options_description description("Allowed options");
    if (description.options().empty())
    {
        description.add_options()
        ("help", "Produce this message")
        ("dllpath", boost::program_options::value<std::string>()->default_value("txmlconnector.dll"), "Path to the transaq dll")
        ("name", boost::program_options::value<std::string>()->default_value("*"), "Username")
        ("pass", boost::program_options::value<std::string>()->default_value("*"), "Password")
        ("host", boost::program_options::value<std::string>()->default_value("213.247.141.133"), "Host")
        ("port", boost::program_options::value<std::string>()->default_value("3900"), "Port")
        ("logdir", boost::program_options::value<std::string>()->default_value("logs"), "Log folder")
        ("loglevel", boost::program_options::value<int32_t>()->default_value(2), "Log level");
    }
    return description;
}

boost::program_options::variables_map parse_options(int argc, char ** argv)
{
    boost::program_options::variables_map options;
    boost::program_options::store
    (
        boost::program_options::parse_command_line
        (
            argc, argv,
            available_options()
        ),
        options
    );
    return options;
}

int main(int argc, char **argv)
{
    try
    {
        setlocale(LC_CTYPE, "");

        boost::program_options::variables_map options = parse_options(argc, argv);
        if (options.count("help"))
        {
            std::cout << available_options() << std::endl;
        }
        else
        {
            transaq::client client(options);
            client.add_strategy("test", boost::shared_ptr<test_strategy>(new test_strategy(client)));
            client.run();
        }
    }
    catch( std::exception & ex )
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
