#pragma once

#include <string>

namespace transaq {

class base_reactor
{
public :
	virtual ~base_reactor() {};
	virtual void handle_data(std::string const& data) = 0;
};

} // namespace transaq
