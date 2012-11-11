#pragma once

#include "types.hpp"
#include "pugixml.hpp"
#include "base_client.hpp"
#include "base_reactor.hpp"
#include <boost/unordered_map.hpp>

namespace transaq {

class base_strategy : public base_reactor
{
public:
	typedef void (*handler_t)(pugi::xml_node const& node, base_strategy & strategy);
	typedef boost::unordered_map<std::string, handler_t> handler_map_t;

public:
	base_strategy(base_client & client);
	
	virtual void handle_data(std::string const& data);
	virtual void handle_info(types::server_status const&) = 0;

protected:
	base_client& client() const { return client_; }

private:
	base_client & client_;
	handler_map_t handlers_;
};

} // namespace transaq
