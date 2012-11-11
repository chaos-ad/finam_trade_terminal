#include "base_strategy.hpp"
#include "pugixml.hpp"

namespace transaq {

/////////////////////////////////////////////////////////////////////////////

void handle_server_status(pugi::xml_node const& node, base_strategy & strategy)
{
	types::server_status value;
	value.id = node.attribute("id").as_int(0);
	value.recover = node.attribute("recover").as_bool(false);
	value.connected = node.attribute("connected").as_bool(false);
	value.message = node.text().as_string();
	strategy.handle_info(value);
}

/////////////////////////////////////////////////////////////////////////////

base_strategy::base_strategy(base_client & client)
	: client_(client)
{
	handlers_["server_status"] = &handle_server_status;
}

void base_strategy::handle_data(std::string const& data)
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
		handler_map_t::const_iterator found = handlers_.find(i->name());
		if (found != handlers_.end())
		{
			handler_t handler = found->second;
			handler(*i, *this);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

} // namespace transaq
