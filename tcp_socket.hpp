#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options/variables_map.hpp>

#include "get_time.hpp"

class tcp_socket
{
public :
    typedef boost::asio::ip::tcp::socket impl_type;

    tcp_socket(boost::asio::io_service & service, boost::program_options::variables_map const& options)
        : impl(service)
    {
        std::string host = options["host"].as<std::string>();
        std::string port = boost::lexical_cast<std::string>(options["port"].as<boost::uint16_t>());
        std::clog << get_time() << ": Connecting to host " << host << ":" << port << "..." << std::endl;
        boost::asio::ip::tcp::resolver::query query(host, port);
        boost::asio::ip::tcp::resolver resolver(service);
        boost::asio::connect(impl, resolver.resolve(query));
        std::clog << get_time() << ": Connected" << std::endl;
    };

    impl_type      & get()       { return impl; }
    impl_type const& get() const { return impl; }

private :
    impl_type impl;
};
