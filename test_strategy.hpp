#pragma once

#include "base_strategy.hpp"

class test_strategy : public transaq::base_strategy
{
public:
	test_strategy(transaq::base_client & client) 
		: transaq::base_strategy(client) 
	{};

	virtual void recovering();
	virtual void disconnected();
    virtual void connected(int32_t id);
};
