#pragma once

#include <string>
#include <cstdint>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace transaq 
{

    struct tick_t
    {
        int32_t                     seqid;
        int64_t                     tradeno;
        boost::posix_time::ptime    tradetime;
        double                      price;
        int32_t                     quantity;
        std::string                 period;
        std::string                 buysell;
        int32_t                     openinterest;
    };
    typedef std::vector<tick>       ticks_t;

} // namespace transaq

