#pragma once

#include "base_strategy.hpp"

class test_strategy : public transaq::base_strategy
{
public:
    typedef transaq::base_strategy base_t;
    using base_t::client;

public:
	test_strategy(transaq::base_client & client) 
		: transaq::base_strategy(client)
		, secid_(-1)
	{};

	virtual void recovering();
	virtual void disconnected();
    virtual void connected(int32_t id);
    
    virtual void ticks(transaq::types::ticks_t const&);
	virtual void markets(transaq::types::markets_t const&);
    virtual void securities(transaq::types::securities_t const&);
    virtual void candlekinds(transaq::types::candlekinds_t const&);

    virtual void client(transaq::types::client_t const&);
    virtual void overnight(bool);

private:
	int32_t secid_;
};
