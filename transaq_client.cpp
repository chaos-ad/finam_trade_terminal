#include <boost/bind.hpp>

#include "transaq_client.hpp"
#include "test_strategy.hpp"

namespace transaq {

/////////////////////////////////////////////////////////////////////////////

client::client(boost::program_options::variables_map const& options)
    : options_(options)
    , sigset_(service_, SIGINT, SIGTERM)
{
    sigset_.async_wait(boost::bind(&client::handle_signal, this, _1, _2));
    wrapper_.reset
    (
        new wrapper
        (
            boost::bind(&client::handle_raw_data, this, _1),
            options_["dllpath"].as<std::string>(),
            options_["logdir"].as<std::string>(),
            options_["loglevel"].as<int32_t>()
        )
    );

    reactors_.push_back(boost::shared_ptr<base_reactor>(new test_strategy(*this)));

	reconnect();
}

void client::run()
{
    service_.run();
}

void client::stop()
{
    service_.stop();
}

/////////////////////////////////////////////////////////////////////////////

void client::handle_signal(boost::system::error_code const& err, int signal_number)
{
    if(!err)
    {
        stop();
    }
}

bool client::handle_raw_data(std::string const& data)
{
    // called from different thread
    service_.post(boost::bind(&client::handle_data, this, data));
    return true;
}

void client::handle_data(std::string const& data)
{
    reactors_t::const_iterator i, end = reactors_.end();
    for( i = reactors_.begin(); i != end; ++i )
    {
        (*i)->handle_data(data);
    }
}

/////////////////////////////////////////////////////////////////////////////

bool client::reconnect()
{
	std::string name = options_["name"].as<std::string>();
    std::string pass = options_["pass"].as<std::string>();
    std::string host = options_["host"].as<std::string>();
    std::string port = options_["port"].as<std::string>();

    std::string command =
    "<command id='connect'>"
        "<login>"    + name + "</login>"
        "<password>" + pass + "</password>"
        "<host>"     + host + "</host>"
        "<port>"     + port + "</port>"
    "</command>";

    return send_command(command);
}

bool client::send_command(std::string const& command)
{
    std::string resposne = wrapper_->send_command(command);
	return resposne == "<result success=\"true\"/>";
}

/////////////////////////////////////////////////////////////////////////////

} // namespace transaq
