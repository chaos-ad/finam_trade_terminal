#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_parsing.hpp>
#include <boost/date_time/date_parsing.hpp>

namespace transaq {
namespace time {

inline boost::posix_time::ptime now()
{
    return boost::posix_time::second_clock::local_time();
}

inline boost::posix_time::ptime parse(std::string const& str)
{
    typedef boost::posix_time::ptime time_type;
    typedef time_type::time_duration_type time_duration;
    typedef time_type::date_type date_type;

    //split date/time on a unique delimiter char such as ' ' or 'T'
    std::string date_string, tod_string;
    boost::date_time::split(str, ' ', date_string, tod_string);
    //call parse_date with first string
    date_type d = boost::date_time::parse_date<date_type>(date_string, boost::date_time::ymd_order_dmy);
    //call parse_time_duration with remaining string
    time_duration td = boost::date_time::parse_delimited_time_duration<time_duration>(tod_string);
    //construct a time
    return time_type(d, td);
}

} // namespace time
} // namespace transaq
