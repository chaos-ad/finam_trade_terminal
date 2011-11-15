#include <iostream>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>

#include "transaq_client.h"

boost::program_options::options_description available_options()
{
    static boost::program_options::options_description description("Allowed options");
    if (description.options().empty())
    {
        description.add_options()
        ("help", "Produce this message")
        ("host", boost::program_options::value<std::string>()->default_value("localhost"), "Connection host")
        ("port", boost::program_options::value<boost::uint16_t>()->default_value(31337), "Connection port")
        ("dllpath", boost::program_options::value<std::string>()->default_value("txmlconnector.dll"), "Path to the transaq dll")
        ("secure", boost::program_options::value<bool>()->default_value(false), "Use SSL")
        ("keyfile", boost::program_options::value<std::string>()->default_value("key.pem"), "Path to SSL key")
        ("certfile", boost::program_options::value<std::string>()->default_value("cert.pem"), "Path to SSL certificate");
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
            transaq::start(options);
        }
    }
    catch( std::exception & ex )
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}