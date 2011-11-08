#include "transaq_wrapper.h"

#include <iostream>
#include <boost/bind.hpp>


// called from different thread, but, who cares
bool handle_data(std::string const& data)
{
    std::cout << data << std::endl;
}


int main(int argc, char **argv)
{
    transaq::wrapper::start(boost::bind(handle_data, _1), "txmlconnector.dll");
    for(;;)
    {
        std::string command;
        std::getline(std::cin, command);
        transaq::wrapper::send_command(command);
    }

    transaq::wrapper::stop();
    return 0;
}
