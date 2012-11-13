#pragma once

#include <cstdint>
#include "base_client.hpp"

namespace transaq {

class base_strategy
{
public:
    base_strategy(base_client & client) : client_(client) {};

	virtual void recovering() = 0;
    virtual void disconnected() = 0;
	virtual void connected(int32_t id) = 0;

	virtual ~base_strategy() {};

protected:
    base_client& client() { return client_; }

private:
    base_client & client_;
};

} // namespace transaq
