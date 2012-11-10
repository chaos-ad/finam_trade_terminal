#include <map>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio/signal_set.hpp>

#include "transaq_client.hpp"
#include "transaq_wrapper.hpp"

namespace transaq {

/////////////////////////////////////////////////////////////////////////////
    
class client::impl
{
public:
    typedef std::map<std::string, std::string> args_t;

public:
    impl(boost::program_options::variables_map const& options)
        : sigset_(service_, SIGINT, SIGTERM)
    {
        sigset_.async_wait(boost::bind(&impl::handle_signal, this, _1, _2));
        wrapper_.reset
        (
            new wrapper
            (
                boost::bind(&impl::handle_raw_data, this, _1), 
                options["dllpath"].as<std::string>(),
                options["logdir"].as<std::string>(),
                options["loglevel"].as<int32_t>()
            )
        );

        args_t args;
        args["login"] = options["name"].as<std::string>();
        args["password"] = options["password"].as<std::string>();
		args["host"] = options["host"].as<std::string>();
		args["port"] = options["port"].as<std::string>();
        send_command("connect", args);
    }

    void run()
    {
        service_.run();
    }

    void stop()
    {
        service_.stop();
    }

private:
    void handle_signal(boost::system::error_code const& err, int signal_number)
    {
        if(!err)
        {
            stop();
        }
    }

    bool handle_raw_data(std::string const& data)
    {
        // called from different thread
        service_.post(boost::bind(&impl::handle_data, this, data));
        return true;
    }

    void handle_data(std::string const& data)
    {
        std::cout << "Data received: " << data << std::endl;
    }

    std::string send_command(std::string const& id, args_t const& args)
    {
        std::string command = "<command id='" + id + "'>";
        args_t::const_iterator i, end = args.end();
        for( i = args.begin(); i != end; ++i )
        {
            command += "<" + i->first + ">";
            command += i->second;
            command += "</" + i->first + ">";
        }
        command += "</command>";
        std::string result = wrapper_->send_command(command);
        handle_data(result);
        return result;
    }

private:
    boost::asio::io_service         service_;
    boost::asio::signal_set         sigset_;
    boost::shared_ptr<wrapper>      wrapper_;
};

/////////////////////////////////////////////////////////////////////////////

client::client(boost::program_options::variables_map const& options)
    : impl_(new impl(options))
{}

void client::run()
{
    impl_->run();
}

void client::stop()
{
	impl_->stop();
}

/////////////////////////////////////////////////////////////////////////////


} // namespace transaq
