#pragma once

#include "base_strategy.hpp"

class test_strategy : public transaq::base_strategy
{
public:
	virtual void recovering();
	virtual void disconnected();
    virtual void connected(int32_t id);
};
