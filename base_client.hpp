#pragma once

#include <map>
#include <cstdint>

namespace transaq {

class base_client
{
public:
    virtual void subscribe_ticks(bool filter, std::map<int32_t, int32_t> const& args) = 0;

protected:
    ~base_client() {};
};

} // namespace transaq
