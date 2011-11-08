#include "transaq_wrapper.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>


boost::mutex mutex;

// called from different thread, but, who cares
bool handle_data(std::string const& data)
{
    boost::lock_guard<boost::mutex> lock(mutex);
    std::cout << data << std::endl;
    return true;
}


int main(int argc, char **argv)
{
    transaq::wrapper::start(boost::bind(handle_data, _1), "txmlconnector.dll");
    for(;;)
    {
        std::string command;
        std::getline(std::cin, command);
        if (command == "exit") break;
        std::string result = transaq::wrapper::send_command(command);
        boost::lock_guard<boost::mutex> lock(mutex);
        std::cout << result << std::endl;
    }

    transaq::wrapper::stop();
    return 0;
}
