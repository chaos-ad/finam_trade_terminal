
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <boost/asio/signal_set.hpp>

#include "get_time.hpp"
#include "locale_bool.hpp"
#include "transaq_client.hpp"
#include "transaq_wrapper.hpp"
#include "libs/pugixml/pugixml.hpp"

namespace transaq {

/////////////////////////////////////////////////////////////////////////////

class client::impl
{
public:
	typedef client::strategy_ptr_t strategy_ptr_t;
    typedef std::map< std::string, strategy_ptr_t> strategies_t;
	typedef strategies_t::const_iterator iter_t;

    typedef void (impl::*handler_t)(pugi::xml_node const& node);
    typedef boost::unordered_map<std::string, handler_t> handlers_t;

public:
    impl(boost::program_options::variables_map const& options)
        : options_(options)
        , sigset_(service_, SIGINT, SIGTERM)
    {
        handlers_["ticks"] = &impl::handle_ticks;
        handlers_["server_status"] = &impl::handle_server_status;

        sigset_.async_wait(boost::bind(&impl::handle_signal, this, _1, _2));
        wrapper_.reset
        (
            new wrapper
            (
                boost::bind(&impl::handle_data, this, _1),
                options_["dllpath"].as<std::string>(),
                options_["logdir"].as<std::string>(),
                options_["loglevel"].as<int32_t>()
            )
        );
        
        connect();
    }

    
    void run()
	{
		service_.run();
	}

    void stop()
	{
		service_.stop();
	}

    void add_strategy(std::string const& name, strategy_ptr_t const& strategy)
	{
		strategies_[name] = strategy;
	}

    void del_strategy(std::string const& name)
	{
		strategies_.erase(name);
	}

private:
    void handle_signal(boost::system::error_code const& err, int signal_number)
    {
        if(!err)
        {
            stop();
        }
    }

    bool handle_data(std::string const& data)
    {
        // called from different thread
        service_.post(boost::bind(&impl::dispatch_data, this, data));
        return true;
    }

    void dispatch_data(std::string const& data)
    {
        pugi::xml_document xml;
        pugi::xml_parse_result res = xml.load(data.c_str());
        if (!res)
        {
            throw std::runtime_error(res.description());
        }

        pugi::xml_node_iterator i, end = xml.end();
        for( i = xml.begin(); i != end; ++i )
        {
            handlers_t::const_iterator found = handlers_.find(i->name());
            if (found != handlers_.end())
            {
                handler_t handler = found->second;
                (this->*handler)(*i);
            }
        }
    }

    void connect()
    {
        std::string name = options_["name"].as<std::string>();
        std::string pass = options_["pass"].as<std::string>();
        std::string host = options_["host"].as<std::string>();
        std::string port = options_["port"].as<std::string>();

		std::clog << "connecting to " << host << ":" << port << " as '" << name << "'..." << std::endl;

		boost::format fmt("<command id=\"connect\"><login>%1%</login><password>%2%</password><host>%3%</host><port>%4%</port></command>");
		send_command( boost::str(fmt % name % pass % host % port) );
    }

    void send_command(std::string const& command)
    {
        std::string resposne = wrapper_->send_command(command);
		pugi::xml_document xml;
        pugi::xml_parse_result res = xml.load(resposne.c_str());
        if (!res)
        {
            throw std::runtime_error(res.description());
        }
		if (!xml.child("result").attribute("success").as_bool())
		{
			throw std::logic_error(xml.child("result").child("message").text().as_string());
		}
    }

/////////////////////////////////////////////////////////////////////////////
public: // base_client api:

    void subscribe_ticks(bool filter, std::map<int32_t, int32_t> const& args)
    {
        boost::format cmd_fmt("<command id=\"subscribe_ticks\" filter=\"%1%\">%2%</command>");
        boost::format args_fmt("<security secid=\"%1%\" tradeno=\"%2%\"/>");
        
        std::string args_str;
        std::map<int32_t, int32_t>::const_iterator i, end = args.end();
        for( i = args.begin(); i != end; ++i )
        {
            args_str += boost::str(args_fmt % i->first % i->second);
        }
        send_command(boost::str(cmd_fmt % locale_bool(filter) % args_str));
    }

/////////////////////////////////////////////////////////////////////////////
private: // packet handlers:

    void handle_server_status(pugi::xml_node const& node)
    {
        if (node.attribute("connected").as_bool(false))
        {
            if (node.attribute("connected").as_bool(false))
            {
                std::clog << get_time() << ": connection lost, recovering..." << std::endl;
				for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
				{
					i->second->recovering();
				}
            }
            else
            {
                int32_t id = node.attribute("id").as_int();
                std::clog << get_time() << ": connected: " << id << std::endl;
				for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
				{
					i->second->connected(id); 
				}
            }
        }
        else
        {
            std::string message = node.text().as_string();
            if(message.empty())
            {
                std::clog << get_time() << ": connection failed" << std::endl;
            }
            else
            {
                std::clog << get_time() << ": connection failed: '" << message << "'" << std::endl;
            }
			for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
			{
				i->second->disconnected(); 
			}
            connect();
        }
    }

    void handle_ticks(pugi::xml_node const& node)
    {
        // TODO: parse ticks here
    }

    /////////////////////////////////////////////////////////////////////////


private:
    boost::program_options::variables_map   options_;
    boost::asio::io_service                 service_;
    boost::asio::signal_set                 sigset_;
    boost::shared_ptr<wrapper>              wrapper_;
    handlers_t                              handlers_;
    strategies_t                            strategies_;

    std::map<int32_t, int32_t>              subscribed_ticks_;
};

/////////////////////////////////////////////////////////////////////////////

client::client(boost::program_options::variables_map const& options)
    : impl_(new impl(options))
{};

client::~client() {};

void client::run() 
{ 
    impl_->run(); 
}

void client::stop() 
{
    impl_->stop(); 
}

void client::add_strategy(std::string const& name, strategy_ptr_t const& strategy)
{
    impl_->add_strategy(name, strategy);
}

void client::del_strategy(std::string const& name)
{
    impl_->del_strategy(name);
}

void client::subscribe_ticks(bool filter, std::map<int32_t, int32_t> const& args)
{
    impl_->subscribe_ticks(filter, args);
}

/////////////////////////////////////////////////////////////////////////////

} // namespace transaq
