#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace transaq {
namespace types {
    
typedef boost::posix_time::ptime ptime;
struct tick_t
{
    int32_t                     secid;
    int64_t                     tradeno;
    ptime                       tradetime;
    double                      price;
    int32_t                     quantity;
    std::string                 period;
    std::string                 buysell;
    int32_t                     openinterest;
};

typedef std::vector<tick_t> ticks_t;
typedef std::map<int32_t, std::string> markets_t;

struct candlekind_t
{
	std::string name;
	std::string period;
};

typedef std::map<int32_t, candlekind_t> candlekinds_t;

struct security_t
{
    int32_t                     secid;
    bool                        active;
    std::string                 seccode;
    int32_t                     market;
    std::string                 shortname;
    int32_t                     decimals;
    double                      minstep;
    int32_t                     lotsize;
    double                      point_cost;
    bool                        usecredit;
    bool                        bymarket;
    bool                        nosplit;
    bool                        immorcancel;
    bool                        cancelbalance;
    std::string                 sectype;
};

typedef std::map<std::string, security_t> securities_t;

struct client_t
{
    std::string                 id;
    bool                        remove;
    std::string                 type;
    std::string                 currency;
    int32_t                     ml_intraday;
    int32_t                     ml_overnight;
    double                      ml_restrict;
    double                      ml_call;
    double                      ml_close;
};

} // namespace types
} // namespace transaq

