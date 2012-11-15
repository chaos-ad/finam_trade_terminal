#include "time.hpp"
#include "test_strategy.hpp"

void test_strategy::recovering()
{
}

void test_strategy::disconnected()
{
}

void test_strategy::connected(int32_t id)
{
    if (secid_ != -1)
    {
        std::map<int32_t, int32_t> args;
        args[secid_] = 0;
        client().subscribe_ticks(true, args);
    }
}

void test_strategy::ticks(transaq::types::ticks_t const& ticks)
{
    std::cout << transaq::time::now() << ": ticks arrived =====" << std::endl;
    transaq::types::ticks_t::const_iterator i, end = ticks.end();
    for( i = ticks.begin(); i != end; ++i )
    {
        std::cout << "    " << i->tradetime << ": price:" << i->price << ", vol: " << i->quantity << std::endl;
    }
}

void test_strategy::markets(transaq::types::markets_t const& markets)
{
    ;
}

void test_strategy::securities(transaq::types::securities_t const& securities)
{
    transaq::types::securities_t::const_iterator i = securities.find("RIZ2");
    if (i != securities.end())
    {
        secid_ = i->second.secid;
    }
}

void test_strategy::candlekinds(transaq::types::candlekinds_t const& candlekinds)
{
    ;
}

void test_strategy::overnight(bool overnight)
{
    ;
}

void test_strategy::client(transaq::types::client_t const& client)
{
    ;
}

void test_strategy::positions(transaq::types::positions_t const& positions)
{
    ;
}
