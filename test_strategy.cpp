#include "test_strategy.hpp"

void test_strategy::recovering()
{
}

void test_strategy::disconnected()
{
	std::map<int32_t, int32_t> args;
    args[93] = 0;
	args[94] = 10;
	args[95] = 100;
    client().subscribe_ticks(true, args);
}

void test_strategy::connected(int32_t id)
{
    std::map<int32_t, int32_t> args;
    args[93] = 0;
    client().subscribe_ticks(true, args);
}
