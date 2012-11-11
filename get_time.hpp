#pragma once

#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

inline boost::posix_time::ptime get_time()
{
    return boost::posix_time::second_clock::local_time();
}
