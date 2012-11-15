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

/////////////////////////////////////////////////////////////////////////////

struct money_position_t
{
    std::vector<int32_t>        markets;
    std::string                 asset;
    std::string                 client;
    std::string                 shortname;
    double                      saldoin;
    double                      bought;
    double                      sold;
    double                      saldo;
    double                      ordbuy;
    double                      ordbycond;
    double                      commission;
};

struct sec_position_t
{
    int32_t                     secid;
    std::string                 client;
    std::string                 shortname;
    int64_t                     saldoin;
    int64_t                     saldomin;
    int64_t                     bought;
    int64_t                     sold;
    int64_t                     saldo;
    int64_t                     ordbuy;
    int64_t                     ordsell;
};

struct forts_position_t
{
    int32_t                     secid;
    std::string                 client;
    int32_t                     startnet;
    int32_t                     openbuys;
    int32_t                     opensells;
    int32_t                     totalnet;
    int32_t                     todaybuy;
    int32_t                     todaysell;
    double                      optmargin;
    double                      varmargin;
    int64_t                     expirationpos;
    double                      usedsellspotlimit;
    double                      sellspotlimit;
    double                      netto;
    double                      kgo;
};

struct forts_money_position_t
{
    std::string                 client;
    std::vector<int32_t>        markets;
    std::string                 shortname;
    double                      current;
    double                      blocked;
    double                      freee;
    double                      varmargin;
};

struct forts_collaterals_t
{
    std::string                 client;
    std::vector<int32_t>        markets;
    std::string                 shortname;
    double                      current;
    double                      blocked;
    double                      freee;
};

struct spot_limit_t
{
    std::string                 client;
    std::vector<int32_t>        markets;
    std::string                 shortname;
    double                      buylimit;
    double                      buylimitused;
};

struct positions_t
{
    std::vector<money_position_t>       money_positions;
    std::vector<sec_position_t>         sec_positions;
    std::vector<forts_position_t>       forts_positions;
    std::vector<forts_money_position_t> forts_money_positions;
    std::vector<forts_collaterals_t>    forts_collaterals;
    std::vector<spot_limit_t>           spot_limits;
};

/////////////////////////////////////////////////////////////////////////////

} // namespace types
} // namespace transaq

