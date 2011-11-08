#pragma once

#include <string>
#include <boost/function.hpp>

namespace transaq
{
    namespace wrapper
    {
        typedef boost::function<bool (std::string const&)> callback_t;

        void stop();
        void start(callback_t const& callback, std::string const& libpath);
        std::string send_command(std::string const& command);
    }
}