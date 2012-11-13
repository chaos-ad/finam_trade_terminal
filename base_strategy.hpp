#pragma once

#include <cstdint>

namespace transaq {

class base_strategy
{
public:
	virtual void recovering() = 0;
    virtual void disconnected() = 0;
	virtual void connected(int32_t id) = 0;

	virtual ~base_strategy() {};
};

} // namespace transaq
