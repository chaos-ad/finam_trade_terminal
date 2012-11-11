#pragma once

#include <string>
#include <cstdint>
#include <boost/optional.hpp>

namespace transaq {
namespace types {

struct server_status
{
	int32_t 	id;
	bool		connected;
	bool 		recover;
	std::string message;
};

} // namespace types
} // namespace transaq
