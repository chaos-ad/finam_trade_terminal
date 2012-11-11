#include <map>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>

#include "base_client.hpp"
#include "base_reactor.hpp"
#include "transaq_wrapper.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace transaq {

/////////////////////////////////////////////////////////////////////////////
    
class client : public base_client
{
public:
	typedef std::list< boost::shared_ptr<base_reactor> > reactors_t;

public:
    client(boost::program_options::variables_map const& options);
    void run();
    void stop();

private:
    void handle_signal(boost::system::error_code const& err, int signal_number);
    bool handle_raw_data(std::string const& data);
    void handle_data(std::string const& data);
    
private: 
	// base_client interface:
	virtual bool reconnect();
	virtual bool send_command(std::string const& command);

private:
	boost::program_options::variables_map 	options_;
    boost::asio::io_service             	service_;
    boost::asio::signal_set             	sigset_;
    boost::shared_ptr<wrapper>          	wrapper_;
	reactors_t								reactors_;
};

/////////////////////////////////////////////////////////////////////////////

} // namespace transaq
