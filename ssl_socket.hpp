#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/program_options/variables_map.hpp>

#include "get_time.hpp"

class ssl_socket
{
public :
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> impl_type;

    ssl_socket(boost::asio::io_service & service, boost::program_options::variables_map const& options)
        : service(service)
        , context(boost::asio::ssl::context::sslv3)
        , impl(service, init_context(context, options))
    {
        std::string host = options["host"].as<std::string>();
        std::string port = boost::lexical_cast<std::string>(options["port"].as<boost::uint16_t>());
        std::clog << get_time() << ": Connecting to host " << host << ":" << port << "..." << std::endl;

        boost::asio::ip::tcp::resolver::query query(host, port);
        boost::asio::ip::tcp::resolver resolver(service);
        boost::asio::connect(impl.lowest_layer(), resolver.resolve(query));

        impl.handshake(boost::asio::ssl::stream_base::client);

        std::clog << get_time() << ": Connected" << std::endl;
    };

private :
    static boost::asio::ssl::context&
    init_context
    (
        boost::asio::ssl::context& context,
        boost::program_options::variables_map const& options
    )
    {
        std::string keyfile = options["keyfile"].as<std::string>();
        std::string certfile = options["certfile"].as<std::string>();
        std::clog << get_time() << ": Using private key: " << keyfile  << std::endl;
        std::clog << get_time() << ": Using certificate: " << certfile << std::endl;

        context.set_verify_mode(get_verify_mode());
        context.use_private_key_file(keyfile, boost::asio::ssl::context::pem);
        context.use_certificate_file(certfile, boost::asio::ssl::context::pem);
        return context;
    }

    static boost::asio::ssl::verify_mode get_verify_mode()
    {
        return
        boost::asio::ssl::verify_peer &
        boost::asio::ssl::verify_fail_if_no_peer_cert;
    }

public :
    impl_type      & get()       { return impl; }
    impl_type const& get() const { return impl; }

private :
    boost::asio::io_service      &  service;
    boost::asio::ssl::context       context;
    impl_type                       impl;
};
