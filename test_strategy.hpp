#pragma once

#include "base_strategy.hpp"

namespace transaq {

class test_strategy : public base_strategy
{
public:
	test_strategy(base_client & client) 
		: base_strategy(client) 
	{};

	virtual void handle_info(types::server_status const& status)
	{
		if (!status.connected)
		{
			client().reconnect();
		}
	}
};

} // namespace transaq
