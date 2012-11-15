
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <boost/asio/signal_set.hpp>

#include "time.hpp"
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
        handlers_["ticks"]            = &impl::handle_ticks;
        handlers_["markets"]        = &impl::handle_markets;
        handlers_["securities"]        = &impl::handle_securities;
        handlers_["candlekinds"]    = &impl::handle_candlekinds;
        handlers_["server_status"]    = &impl::handle_server_status;
        handlers_["client"]         = &impl::handle_client;
        handlers_["positions"]      = &impl::handle_positions;
        handlers_["overnight"]      = &impl::handle_overnight;

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
            else
            {
                std::cout << time::now() << ": unhandled info: " << i->name() << std::endl;
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
            if (node.attribute("recover").as_bool(false))
            {
                std::clog << time::now() << ": connection lost, recovering..." << std::endl;
                for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
                {
                    i->second->recovering();
                }
            }
            else
            {
                int32_t id = node.attribute("id").as_int();
                std::clog << time::now() << ": connected: " << id << std::endl;
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
                std::clog << time::now() << ": connection failed" << std::endl;
            }
            else
            {
                std::clog << time::now() << ": connection failed: '" << message << "'" << std::endl;
            }
            for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
            {
                i->second->disconnected(); 
            }
            connect();
        }
    }

    void handle_markets(pugi::xml_node const& node)
    {
        std::map<int32_t, std::string> markets;
        pugi::xml_node_iterator i, end = node.end();
        for( i = node.begin(); i != end; ++i )
        {
            markets[i->attribute("id").as_int()] = i->text().as_string();
        }
        if (!markets.empty())
        {
            for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
            {
                i->second->markets(markets); 
            }
        }
    }

    void handle_securities(pugi::xml_node const& node)
    {
        types::securities_t securities;
        pugi::xml_node_iterator i, end = node.end();
        for( i = node.begin(); i != end; ++i )
        {
            types::security_t security;
            security.secid = i->attribute("secid").as_int();
            security.active = i->attribute("active").as_bool();
            security.seccode = i->child("seccode").text().as_string();
            security.market = i->child("market").text().as_int();
            security.shortname = i->child("shortname").text().as_string();
            security.decimals = i->child("decimals").text().as_int();
            security.minstep = i->child("minstep").text().as_double();
            security.lotsize = i->child("lotsize").text().as_int();
            security.point_cost = i->child("point_cost").text().as_double();
            pugi::xml_node opmask = i->child("opmask");
            security.usecredit = opmask.attribute("usecredit").as_bool(false);
            security.bymarket = opmask.attribute("bymarket").as_bool(false);
            security.nosplit = opmask.attribute("nosplit").as_bool(false);
            security.immorcancel = opmask.attribute("immorcancel").as_bool(false);
            security.cancelbalance = opmask.attribute("cancelbalance").as_bool(false);
            security.sectype = i->child("sectype").text().as_string();
            securities[security.seccode] = security;
        }
        if (!securities.empty())
        {
            for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
            {
                i->second->securities(securities); 
            }
        }
    }

    void handle_candlekinds(pugi::xml_node const& node)
    {
        types::candlekinds_t candlekinds;
        pugi::xml_node_iterator i, end = node.end();
        for( i = node.begin(); i != end; ++i )
        {
            types::candlekind_t kind;
            int32_t id = i->child("id").text().as_int();
            kind.name =    i->child("name").text().as_string();
            kind.period = i->child("period").text().as_string();
            candlekinds[id] = kind;
        }
        if (!candlekinds.empty())
        {
            for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
            {
                i->second->candlekinds(candlekinds); 
            }
        }
    }

    void handle_ticks(pugi::xml_node const& node)
    {
        types::ticks_t ticks;
        pugi::xml_node_iterator i, end = node.end();
        for( i = node.begin(); i != end; ++i )
        {
            types::tick_t tick;
            tick.secid = i->child("secid").text().as_int();
            tick.tradeno = boost::lexical_cast<int64_t>(i->child("tradeno").text().as_string());
            std::string tradetime = i->child("tradetime").text().as_string();
            tick.tradetime = time::parse(tradetime);
            tick.price = boost::lexical_cast<double>(i->child("price").text().as_string());
            tick.quantity = i->child("quantity").text().as_int();
            tick.period = i->child("period").text().as_string();
            tick.buysell = i->child("buysell").text().as_string();
            tick.openinterest = i->child("openinterest").text().as_int();
            ticks.push_back(tick);
        }
        if (!ticks.empty())
        {
            for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
            {
                i->second->ticks(ticks); 
            }
        }
    }

    void handle_client(pugi::xml_node const& node)
    {
        types::client_t client;
        client.id = node.attribute("id").as_string();
        client.remove = node.attribute("remove").as_bool();
        client.currency = node.child("currency").text().as_string();
        client.type = node.child("type").text().as_string();
        client.ml_intraday = node.child("ml_intraday").text().as_int();
        client.ml_overnight = node.child("ml_overnight").text().as_int();
        client.ml_restrict = node.child("ml_restrict").text().as_double();
        client.ml_call = node.child("ml_call").text().as_double();
        client.ml_close = node.child("ml_close").text().as_double();
        for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
        {
            i->second->client(client); 
        }
    }

    void handle_positions(pugi::xml_node const& node)
    {
        types::positions_t positions;
        pugi::xml_node_iterator i, end = node.end();
        for( i = node.begin(); i != end; ++i )
        {
            pugi::xml_node p = *i;
            std::string type = i->name();
            if (type == "money_position") 
            {
                types::money_position_t pos;
                pugi::xml_node_iterator j, jend = p.child("markets").end();
                for( j = p.child("markets").begin(); j != jend; ++j )
                {
                    pos.markets.push_back(j->text().as_int());
                }
                pos.asset = p.child("asset").text().as_string();
                pos.client = p.child("client").text().as_string();
                pos.shortname = p.child("shortname").text().as_string();
                pos.saldoin = p.child("saldoin").text().as_double();
                pos.bought = p.child("bought").text().as_double();
                pos.sold = p.child("sold").text().as_double();
                pos.saldo = p.child("saldo").text().as_double();
                pos.ordbuy = p.child("ordbuy").text().as_double();
                pos.ordbycond = p.child("ordbycond").text().as_double();
                pos.commission = p.child("commission").text().as_double();
                positions.money_positions.push_back(pos);
            }
            else if (type == "sec_position") 
            {
                types::sec_position_t pos;
                pos.secid = p.child("secid").text().as_int();
                pos.client = p.child("client").text().as_string();
                pos.shortname = p.child("shortname").text().as_string();
                pos.saldoin = boost::lexical_cast<int64_t>( p.child("saldoin").text().as_string() );
                pos.saldomin = boost::lexical_cast<int64_t>( p.child("saldomin").text().as_string() );
                pos.bought = boost::lexical_cast<int64_t>( p.child("bought").text().as_string() );
                pos.sold = boost::lexical_cast<int64_t>( p.child("sold").text().as_string() );
                pos.saldo = boost::lexical_cast<int64_t>( p.child("saldo").text().as_string() );
                pos.ordbuy = boost::lexical_cast<int64_t>( p.child("ordbuy").text().as_string() );
                pos.ordsell = boost::lexical_cast<int64_t>( p.child("ordsell").text().as_string() );
                positions.sec_positions.push_back(pos);
            }
            else if (type == "forts_position") 
            {
                types::forts_position_t pos;
                pos.secid = p.child("secid").text().as_int();
                pos.client = p.child("client").text().as_string();
                pos.startnet = p.child("startnet").text().as_int();
                pos.openbuys = p.child("openbuys").text().as_int();
                pos.opensells = p.child("opensells").text().as_int();
                pos.totalnet = p.child("totalnet").text().as_int();
                pos.todaybuy = p.child("todaybuy").text().as_int();
                pos.todaysell = p.child("todaysell").text().as_int();
                pos.optmargin = p.child("optmargin").text().as_double();
                pos.varmargin = p.child("varmargin").text().as_double();
                pos.expirationpos = boost::lexical_cast<int64_t>( p.child("expirationpos").text().as_string() );
                pos.usedsellspotlimit = p.child("usedsellspotlimit").text().as_double();
                pos.sellspotlimit = p.child("sellspotlimit").text().as_double();
                pos.netto = p.child("netto").text().as_double();
                pos.kgo = p.child("kgo").text().as_double();
                positions.forts_positions.push_back(pos);
            }
            else if (type == "forts_money") 
            {
                types::forts_money_position_t pos;
                pugi::xml_node_iterator j, jend = p.child("markets").end();
                for( j = p.child("markets").begin(); j != jend; ++j )
                {
                    pos.markets.push_back(j->text().as_int());
                }
                pos.client = p.child("client").text().as_string();
                pos.shortname = p.child("shortname").text().as_string();
                pos.current = p.child("current").text().as_double();
                pos.blocked = p.child("blocked").text().as_double();
                pos.freee = p.child("freee").text().as_double();
                pos.varmargin = p.child("varmargin").text().as_double();
                positions.forts_money_positions.push_back(pos);
            }
            else if (type == "forts_collaterals") 
            {
                types::forts_collaterals_t pos;
                pugi::xml_node_iterator j, jend = p.child("markets").end();
                for( j = p.child("markets").begin(); j != jend; ++j )
                {
                    pos.markets.push_back(j->text().as_int());
                }
                pos.client = p.child("client").text().as_string();
                pos.shortname = p.child("shortname").text().as_string();
                pos.current = p.child("current").text().as_double();
                pos.blocked = p.child("blocked").text().as_double();
                pos.freee = p.child("freee").text().as_double();
                positions.forts_collaterals.push_back(pos);
            }
            else if (type == "spot_limit")
            {
                types::spot_limit_t pos;
                pugi::xml_node_iterator j, jend = p.child("markets").end();
                for( j = p.child("markets").begin(); j != jend; ++j )
                {
                    pos.markets.push_back(j->text().as_int());
                }
                pos.client = p.child("client").text().as_string();
                pos.shortname = p.child("shortname").text().as_string();
                pos.buylimit = p.child("buylimit").text().as_double();
                pos.buylimitused = p.child("buylimitused").text().as_double();
                positions.spot_limits.push_back(pos);
            }
        }
        for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
        {
            i->second->positions(positions); 
        }
    }

    void handle_overnight(pugi::xml_node const& node)
    {
        for( iter_t i = strategies_.begin(), end = strategies_.end(); i != end; ++i )
        {
            i->second->overnight(node.attribute("status").as_bool()); 
        }
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
