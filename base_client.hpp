#pragma once

#include <string>

namespace transaq {

class base_client
{
public:
	virtual ~base_client() {};
	virtual bool reconnect() = 0;
	virtual bool send_command(std::string const& command) = 0;
};

} // namespace transaq
